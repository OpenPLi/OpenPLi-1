/* 
 * dvb_net.c
 *
 * Copyright (C) 2001 Convergence integrated media GmbH
 *                    Ralph Metzler <ralph@convergence.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifdef __DVB_PACK__
#include <ost/demux.h>
#else
#include <linux/ost/demux.h>
#endif

#include "dvb_net.h"

#ifdef MODULE
MODULE_DESCRIPTION("DVB network driver");
MODULE_AUTHOR("Ralph Metzler");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#endif

/* external stuff in dvb.c */
int register_dvbnet(dvb_net_t *dvbnet);
int unregister_dvbnet(dvb_net_t *dvbnet);

/* internal stuff */
dvb_net_t dvb_net;

/*
 *	Determine the packet's protocol ID. The rule here is that we 
 *	assume 802.3 if the type field is short enough to be a length.
 *	This is normal practice and works for any 'now in use' protocol.
 *
 *  stolen from eth.c out of the linux kernel, hacked for dvb-device
 *  by Michael Holzt <kju@debian.org>
 */
 
unsigned short my_eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	struct ethhdr *eth;
	unsigned char *rawp;
	
	skb->mac.raw=skb->data;
	skb_pull(skb,dev->hard_header_len);
	eth= skb->mac.ethernet;
	
	if(*eth->h_dest&1)
	{
		if(memcmp(eth->h_dest,dev->broadcast, ETH_ALEN)==0)
			skb->pkt_type=PACKET_BROADCAST;
		else
			skb->pkt_type=PACKET_MULTICAST;
	}
	
	if (ntohs(eth->h_proto) >= 1536)
		return eth->h_proto;
		
	rawp = skb->data;
	
	/*
	 *	This is a magic hack to spot IPX packets. Older Novell breaks
	 *	the protocol design and runs IPX over 802.3 without an 802.2 LLC
	 *	layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
	 *	won't work for fault tolerant netware but does for the rest.
	 */
	if (*(unsigned short *)rawp == 0xFFFF)
		return htons(ETH_P_802_3);
		
	/*
	 *	Real 802.2 LLC
	 */
	return htons(ETH_P_802_2);
}

static void 
dvb_net_sec(struct net_device *dev, u8 *pkt, int pkt_len)
{
        u8 *eth;
        struct sk_buff *skb;

        if (!pkt_len)
                return;
        skb = dev_alloc_skb(pkt_len+2);
        if (skb == NULL) {
                printk(KERN_NOTICE "%s: Memory squeeze, dropping packet.\n",
                       dev->name);
                ((dvb_net_priv_t *)dev->priv)->stats.rx_dropped++;
                return;
        }
        eth=(u8 *) skb_put(skb,pkt_len+2);
        memcpy(eth+14, (void*)pkt+12, pkt_len-12);

        eth[0]=pkt[0x0b];
        eth[1]=pkt[0x0a];
        eth[2]=pkt[0x09];
        eth[3]=pkt[0x08];
        eth[4]=pkt[0x04];
        eth[5]=pkt[0x03];
        eth[6]=eth[7]=eth[8]=eth[9]=eth[10]=eth[11]=0;
        eth[12]=0x08; eth[13]=0x00;

	skb->protocol=my_eth_type_trans(skb,dev);
        skb->dev=dev;
        
        ((dvb_net_priv_t *)dev->priv)->stats.rx_packets++;
        ((dvb_net_priv_t *)dev->priv)->stats.rx_bytes+=skb->len;
        sti();
        netif_rx(skb);
}
 
static int 
dvb_net_callback(u8 *buffer1, size_t buffer1_len,
		 u8 *buffer2, size_t buffer2_len,
		 dmx_section_filter_t *filter,
		 dmx_success_t success)
{
        struct net_device *dev=(struct net_device *) filter->priv;

	/* FIXME: this only works if exactly one complete section is
	          delivered in buffer1 only */
	dvb_net_sec(dev, buffer1, buffer1_len);
	return 0;
}

static int
dvb_net_tx(struct sk_buff *skb, struct net_device *dev)
{
	return 0;
}

static u8 mask_normal[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static u8 mask_allmulti[6]={0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
static u8 mac_allmulti[6]={0x01, 0x00, 0x5e, 0x00, 0x00, 0x00};
static u8 mask_promisc[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int 
dvb_net_filter_set(struct net_device *dev, 
		   dmx_section_filter_t **secfilter,
		   unsigned char *mac, u8 *mac_mask)
{
	dvb_net_priv_t *priv=(dvb_net_priv_t *)dev->priv;
	int ret;

	*secfilter=0;
	ret=priv->secfeed->allocate_filter(priv->secfeed, secfilter);
	if (ret<0) {
		printk("%s: could not get filter\n", dev->name);
		return ret;
	}

	(*secfilter)->priv=(void *) dev;

	memset((*secfilter)->filter_value, 0, DMX_MAX_FILTER_SIZE);
	memset((*secfilter)->filter_mask , 0, DMX_MAX_FILTER_SIZE);

	(*secfilter)->filter_value[0]=0x3e;
	(*secfilter)->filter_mask[0]=0xff;

	(*secfilter)->filter_value[3]=mac[5];
	(*secfilter)->filter_mask[3]=mac_mask[5];
	(*secfilter)->filter_value[4]=mac[4];
	(*secfilter)->filter_mask[4]=mac_mask[4];
	(*secfilter)->filter_value[8]=mac[3];
	(*secfilter)->filter_mask[8]=mac_mask[3];
	(*secfilter)->filter_value[9]=mac[2];
	(*secfilter)->filter_mask[9]=mac_mask[2];

	(*secfilter)->filter_value[10]=mac[1];
	(*secfilter)->filter_mask[10]=mac_mask[1];
	(*secfilter)->filter_value[11]=mac[0];
	(*secfilter)->filter_mask[11]=mac_mask[0];

	printk("%s: filter mac=%02x %02x %02x %02x %02x %02x\n", 
	       dev->name, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return 0;
}

static int
dvb_net_feed_start(struct net_device *dev)
{
	int ret, i;
	dvb_net_priv_t *priv=(dvb_net_priv_t *)dev->priv;
        dmx_demux_t *demux=priv->demux;
        unsigned char *mac=(unsigned char *) dev->dev_addr;
		
	priv->secfeed=0;
	priv->secfilter=0;

	ret=demux->allocate_section_feed(demux, &priv->secfeed, 
					 dvb_net_callback);
	if (ret<0) {
		printk("%s: could not get section feed\n", dev->name);
		return ret;
	}

	ret=priv->secfeed->set(priv->secfeed, priv->pid, 32768, 0, 0);
	if (ret<0) {
		printk("%s: could not set section feed\n", dev->name);
		priv->demux->
		        release_section_feed(priv->demux, priv->secfeed);
		priv->secfeed=0;
		return ret;
	}
	MOD_INC_USE_COUNT;
	
	if (priv->mode<3) 
		dvb_net_filter_set(dev, &priv->secfilter, mac, mask_normal);

	switch (priv->mode) {
	case 1:
		for (i=0; i<priv->multi_num; i++) 
			dvb_net_filter_set(dev, &priv->multi_secfilter[i],
					   priv->multi_macs[i], mask_normal);
		break;
	case 2:
		priv->multi_num=1;
		dvb_net_filter_set(dev, &priv->multi_secfilter[0], mac_allmulti, mask_allmulti);
		break;
	case 3:
		priv->multi_num=0;
		dvb_net_filter_set(dev, &priv->secfilter, mac, mask_promisc);
		break;
	}
	
	priv->secfeed->start_filtering(priv->secfeed);
	return 0;
}

static void
dvb_net_feed_stop(struct net_device *dev)
{
	dvb_net_priv_t *priv=(dvb_net_priv_t *)dev->priv;
	int i;

        if (priv->secfeed) {
	        if (priv->secfeed->is_filtering)
		        priv->secfeed->stop_filtering(priv->secfeed);
	        if (priv->secfilter)
		        priv->secfeed->
			        release_filter(priv->secfeed, 
					       priv->secfilter);
		priv->secfilter=0;

		for (i=0; i<priv->multi_num; i++) {
			if (priv->multi_secfilter[i])
				priv->secfeed->
					release_filter(priv->secfeed, 
						       priv->multi_secfilter[i]);
			priv->multi_secfilter[i]=0;
		}
		priv->demux->
		        release_section_feed(priv->demux, priv->secfeed);
		priv->secfeed=0;
		MOD_DEC_USE_COUNT;
	} else
		printk("%s: no feed to stop\n", dev->name);
}

static int
dvb_set_mc_filter(struct net_device *dev, struct dev_mc_list *mc)
{
	dvb_net_priv_t *priv=(dvb_net_priv_t *)dev->priv;

	if (priv->multi_num==DVB_NET_MULTICAST_MAX)
		return -ENOMEM;

	memcpy(priv->multi_macs[priv->multi_num], mc->dmi_addr, 6);
	
	priv->multi_num++;
	return 0;
}

static void
dvb_net_set_multi(struct net_device *dev)
{
	dvb_net_priv_t *priv=(dvb_net_priv_t *)dev->priv;

	dvb_net_feed_stop(dev);
	
	priv->mode=0;
	if (dev->flags&IFF_PROMISC) {
		printk("%s: promiscuous mode\n", dev->name);
		priv->mode=3;
	} else if((dev->flags&IFF_ALLMULTI)) {
		printk("%s: allmulti mode\n", dev->name);
		priv->mode=2;
	} else if(dev->mc_count) {
                int mci;
                struct dev_mc_list *mc;
		
		printk("%s: set_mc_list, %d entries\n", 
		       dev->name, dev->mc_count);
		priv->mode=1;
		priv->multi_num=0;
                for (mci=0, mc=dev->mc_list; 
		     mci<dev->mc_count;
		     mc=mc->next, mci++) {
			dvb_set_mc_filter(dev, mc);
                } 
	}
	dvb_net_feed_start(dev);
}

static int
dvb_net_set_config(struct net_device *dev, struct ifmap *map)
{
	if (netif_running(dev))
		return -EBUSY;
	return 0;
}

static int
dvb_net_set_mac(struct net_device *dev, void *p)
{
	struct sockaddr *addr=p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	if (netif_running(dev)) {
	        dvb_net_feed_stop(dev);
		dvb_net_feed_start(dev);
	}
        return 0;
}


static int
dvb_net_open(struct net_device *dev)
{
	dvb_net_feed_start(dev);
	return 0;
}

static int
dvb_net_stop(struct net_device *dev)
{
        dvb_net_feed_stop(dev);
	return 0;
}

static struct net_device_stats *
dvb_net_get_stats(struct net_device *dev)
{
        return &((dvb_net_priv_t *)dev->priv)->stats;
}


static int
dvb_net_init_dev(struct net_device *dev)
{
	ether_setup(dev);

	dev->open		= dvb_net_open;
	dev->stop		= dvb_net_stop;
	dev->hard_start_xmit	= dvb_net_tx;
	dev->get_stats		= dvb_net_get_stats;
	dev->set_multicast_list = dvb_net_set_multi;
	dev->set_config         = dvb_net_set_config;
	dev->set_mac_address    = dvb_net_set_mac;
	dev->mtu		= 4096;
	dev->mc_count=0;

	dev->flags             |= IFF_NOARP;
	dev->hard_header_cache  = NULL;
	
	return 0;
}

static int 
get_if(dvb_net_t *dvbnet)
{
	int i;

	for (i=0; i<dvbnet->dev_num; i++) 
		if (!dvbnet->state[i])
			break;
	if (i==dvbnet->dev_num)
		return -1;
	dvbnet->state[i]=1;
	return i;
}


int 
dvb_net_add_if(dvb_net_t *dvbnet, u16 pid)
{
        struct net_device *net;
	dmx_demux_t *demux;
	dvb_net_priv_t *priv;
	int result;
	int if_num;
 
	if_num=get_if(dvbnet);
	if (if_num<0)
		return -EINVAL;

	net=&dvbnet->device[if_num];
	demux=dvbnet->demux;
	
	net->base_addr = 0;
	net->irq       = 0;
	net->dma       = 0;
	net->mem_start = 0;
        memcpy(net->name, "dvb0_0", 7);
        net->name[3]=dvbnet->card_num+0x30;
        net->name[5]=if_num+0x30;
        net->next      = NULL;
        net->init      = dvb_net_init_dev;
        net->priv      = kmalloc(sizeof(dvb_net_priv_t), GFP_KERNEL);
	if (net->priv == NULL)
			return -ENOMEM;

	priv=net->priv;
	memset(priv, 0, sizeof(dvb_net_priv_t));
        priv->demux=demux;
        priv->pid=pid;
	priv->mode=0;

        net->base_addr=pid;
                
	if ((result = register_netdev(net)) < 0) {
		return result;
	}
	MOD_INC_USE_COUNT;
        return if_num;
}

int 
dvb_net_remove_if(dvb_net_t *dvbnet, int num)
{
	if (!dvbnet->state[num])
		return -EINVAL;
	dvb_net_stop(&dvbnet->device[num]);
        kfree(dvbnet->device[num].priv);
        unregister_netdev(&dvbnet->device[num]);
	dvbnet->state[num]=0;
	MOD_DEC_USE_COUNT;
	return 0;
}

void
dvb_net_release(dvb_net_t *dvbnet)
{
	int i;

	for (i=0; i<dvbnet->dev_num; i++) {
		if (!dvbnet->state[i])
			continue;
		dvb_net_remove_if(dvbnet, i);
	}
}

int
dvb_net_init(dvb_net_t *dvbnet, dmx_demux_t *demux)
{
	int i;
		
	dvbnet->demux=demux;
	dvbnet->dev_num=DVB_NET_DEVICES_MAX;
	for (i=0; i<dvbnet->dev_num; i++) 
		dvbnet->state[i]=0;
	return 0;
}

#ifdef MODULE

int
init_module (void)
{
	printk("dvb_net: $Id: dvb_net.c,v 1.6 2002/08/12 18:24:05 obi Exp $\n");

	dvb_net.dvb_net_release   = dvb_net_release;
	dvb_net.dvb_net_init      = dvb_net_init;
	dvb_net.dvb_net_add_if    = dvb_net_add_if;
	dvb_net.dvb_net_remove_if = dvb_net_remove_if;

	return register_dvbnet(&dvb_net);
}

void
cleanup_module(void)
{
	unregister_dvbnet(&dvb_net);
}

#endif /* MODULE */


#include <src/rds_text.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#include <src/enigma.h>
#include <lib/dvb/decoder.h>
#include <lib/driver/streamwd.h>

#include <errno.h>

#define SWAP(x)	((x<<8)|(x>>8))
#define LO(x)	(x&0xFF)

static inline unsigned short crc_ccitt_byte( unsigned short crc, unsigned char c )
{
	crc = SWAP(crc) ^ c;
	crc = crc ^ (LO(crc) >> 4);
	crc = crc ^ (SWAP(LO(crc)) << 4) ^ (LO(crc) << 5);
	return crc;
}

RDSTextDecoder::RDSTextDecoder()
	: m_interactive(0), wasVisible(0), bytesread(0), ptr(0), p1(-1), p2(-1), qdar_pos(0), rass_imode_active(0), leninfo(0), text_len(0), m_ptr(0), state(0)
	, is_sync(0), paket_size(0), sync_try(0), sn(0), rass_logo( eZap::getInstance()->getDesktop( eZap::desktopFB )), qdarmvi_show(-1)
{
	init_RDSTextDecoder();
}
void RDSTextDecoder::init_RDSTextDecoder()
{
	int fd=open("/dev/dvb/card0/ancillary0", O_RDONLY|O_NONBLOCK );
	if ( fd < 0 )
		eDebug("open /dev/dvb/card0/ancillary0 failed(%m)");
	else
	{
		sn = new eSocketNotifier(eApp, fd, eSocketNotifier::Read);
		CONNECT(sn->activated, RDSTextDecoder::process_data);
	}

	memset(rtp_item, 0, sizeof(rtp_item));

	int x = eSkin::getActive()->queryValue("rds.pos.x", 0);
	int y = eSkin::getActive()->queryValue("rds.pos.y", 0);
	int width = eSkin::getActive()->queryValue("rds.pos.width", 0);
	int height = eSkin::getActive()->queryValue("rds.pos.height", 0);
	gFont rds_font = eSkin::getActive()->queryFont("rds");

	// OSD Radiotext
	rds_text = new eLabel( eZap::getInstance()->getDesktop( eZap::desktopFB ) );
	rds_text->move(ePoint(x, y));
	rds_text->resize(eSize(width, height));
	rds_text->setFont(rds_font);
	rds_text->setProperty("align","left");
	rds_text->setProperty("backgroundColor","rds_bg");
	rds_text->setProperty("foregroundColor","rds_fg");
	rds_text->hide();

	x = eSkin::getActive()->queryValue("rds_plus.pos.x", 0);
	y = eSkin::getActive()->queryValue("rds_plus.pos.y", 0);
	width = eSkin::getActive()->queryValue("rds_plus.pos.width", 0);
	height = eSkin::getActive()->queryValue("rds_plus.pos.height", 0);
	
	// OSD RTPlus Text
	rtp_text = new eLabel( eZap::getInstance()->getDesktop( eZap::desktopFB ) );
	rtp_text->move(ePoint(x, y));
	rtp_text->resize(eSize(width, height));
	rtp_text->setFont(rds_font);
	rtp_text->setProperty("align","right");
	rtp_text->setProperty("backgroundColor","rds_bg");
	rtp_text->setProperty("foregroundColor","rds_fg");
	rtp_text->hide();

	x = eSkin::getActive()->queryValue("rass_logo.pos.x", 0);
	y = eSkin::getActive()->queryValue("rass_logo.pos.y", 0);
	
	// Rass Logo
	gPixmap *pm = eSkin::getActive()->queryImage("rass_logo");
	rass_logo.setPixmap(pm);
	rass_logo.move( ePoint(x, y) );
	rass_logo.resize( eSize(50, 21) );
	rass_logo.pixmap_position = ePoint(0,0);
	rass_logo.hide();
	CONNECT(eWidget::globalFocusChanged, RDSTextDecoder::globalFocusHasChanged);
}

RDSTextDecoder::~RDSTextDecoder()
{
	int fd = sn ? sn->getFD() : -1;
	if ( fd != -1 )
	{
		delete sn;
		close(fd);
	}
}

void RDSTextDecoder::clear_service()
{
	is_sync=0;paket_size=0;sync_try=0;
	state=bytesread=ptr=0;
	p1=p2=-1;
	message[0]=lastmessage[0]=0;

	int tmp=0;
	while(tmp++ < 64)
		rtp_item[tmp][0]=0;
	
	qdarmvi_show=interactive_avail=0;
	rds_text->hide();
	rtp_text->hide();
	rass_logo.hide();
	wasVisible &= ~(1|2|4);
	
	// delete cached rass slides
	DIR *tmp_directory;
	struct dirent *filename;
	char tmp_filename[128];

	tmp_directory = opendir("/tmp");
	while ((filename = readdir(tmp_directory)) != NULL)
	{
		if (strstr(filename->d_name,"Rass") && strstr(filename->d_name,".mvi"))
		{
			sprintf(tmp_filename,"/tmp/%s",filename->d_name);
			remove(tmp_filename);
		}
	}
	closedir(tmp_directory);
}

void RDSTextDecoder::process_qdar(unsigned char *buf)
{
	if (buf[0] == 0x40 && buf[1] == 0xDA)
	{
		int item,cnt,ctrl,item_type;
		long item_length,id,item_no,ptr,tmp;
		unsigned short crc_qdar,crc_read;
		char fname[50];
		ptr=4;cnt=0;
		item=buf[2]<<8; // Number of Items
		item|=buf[3];
		
		while ( cnt++ < item ) //read in items
		{
			id=buf[ptr++]<<8; //QDarID
			id|=buf[ptr++];
			
			item_no=buf[ptr++]<<8; // Item Number
			item_no|=buf[ptr++];
			
			ctrl=buf[ptr++]; //controlbyte
			item_type=buf[ptr++]; //item type
			
			item_length=buf[ptr++]<<24; // Item length
			item_length|=buf[ptr++]<<16;
			item_length|=buf[ptr++]<<8;
			item_length|=buf[ptr++];
			
			ptr=ptr+4; // rfu Bytes ... not used
			tmp=ptr; // calc crc
			crc_qdar=0xFFFF;
			while (tmp < ptr+item_length)
				crc_qdar = crc_ccitt_byte(crc_qdar, buf[tmp++]);
		
			crc_read=buf[ptr+item_length]<<8;
			crc_read|=buf[ptr+item_length+1];
			//eDebug("[RDS/Rass] CRC read: %04X calculated: %04X",crc_read,crc_qdar^0xFFFF);
			
			if (crc_read == (crc_qdar^0xFFFF)) // process item
			{
				switch(item_type)
				{
					case 0x01: //Stillframe
						if (ctrl&0x01) // display slide
						{
							if (!qdarmvi_show)
								eStreamWatchdog::getInstance()->reloadSettings(3); // set mpeg decoder to 16:9
							qdarmvi_show=1;
							if (rass_imode_active != 1)
								Decoder::displayIFrame((const char*)buf+ptr,item_length-2);
							// save as last slide, used for leaving interactive mode
							sprintf(fname,"/tmp/RassLast.mvi");
							FILE *fh=fopen(fname,"wb");
							fwrite(buf+ptr,1,item_length-2,fh);
							fclose(fh);
						}
						if (ctrl&0x02) // save slide for interactive mode
						{
							sprintf(fname,"/tmp/Rass%04d.mvi",(int)id);
							FILE *fh=fopen(fname,"wb");
							fwrite(buf+ptr,1,item_length-2,fh);
							fclose(fh);
							if (id == 0 && interactive_avail == 0)
							{
								interactive_avail=1;
								if (wasVisible & 8) // another widget/window visible
									wasVisible |= 4;
								else
									rass_logo.show();
							}
							if (m_interactive && id != 0)
								m_interactive->update_avail((int)(id/1000));
						}
						if (ctrl&0x04) // display slide if nothing had been displayed yet
						{
							if (qdarmvi_show != 1)
							{
								eStreamWatchdog::getInstance()->reloadSettings(3); // set mpeg decoder to 16:9
								qdarmvi_show=1;
								if (rass_imode_active != 1)
									Decoder::displayIFrame((const char*)buf+ptr,item_length-2);
								// save as last slide, used for leaving interactive mode
								sprintf(fname,"/tmp/RassLast.mvi");
								FILE *fh=fopen(fname,"wb");
								fwrite(buf+ptr,1,item_length-2,fh);
								fclose(fh);
							}
						}						
						if (ctrl&0x08) // delete slide
						{
							sprintf(fname,"/tmp/Rass%04d.mvi",(int)id);
							remove(fname);
						}						
						break;
					default: //nothing more yet defined
						break;
				}
			} 
			else
			{
				eDebug("[RDS/Rass] CRC error, skip Rass-Qdar-Item");
			}
			
			ptr=+item_length;
		}
	}
	else
	{
		eDebug("[RDS/Rass] No Rass-QDAR archive (%02X %02X) so skipping !\n",buf[0],buf[1]);
	}
}

void RDSTextDecoder::process_data(int what)
{
	int rd=read(sn->getFD(), buf+bytesread, 263-bytesread);
	if ( rd >= 0 )
	{
		bytesread+=rd;
		if ( bytesread == 263 )
		{
			while(ptr<263)
			{
				// sync ancillary stream 
				if (sync_try > 99) // sync failed after 100 tries ? give up and spend the cpu time for something useful ;)
				{
					eDebug("[RDS/Rass] ancillary sync not found (%d) ... give up",sync_try);
					bytesread=0;ptr=0;
					return;
				}
				if (is_sync < 5) // try to get paket_length for sync ancillary stream
				{
					if ( buf[ptr] == 0xFD && ptr > p1 )
					{
						if ( sync_try < 15 || buf[ptr-1] > 0x02 )
						{
							if (p1 == -1)
								p1=ptr;
							else
								p2=ptr;
						}
					}
					if ( p1 != -1 && p2 != -1 )
					{
						++sync_try;
						if ((paket_size == p2-p1) || (paket_size == (p2-p1)+2) || (paket_size == (p2-p1)-2))
							++is_sync;
						else
						{
							paket_size=p2-p1;
							is_sync=0;
						}
						if (is_sync < 5)
						{
							p1=ptr;
							p2=-1;
						}
						else
							eDebug("[RDS/Rass] ancillary sync found (%d/%d) ...",paket_size,sync_try);
					}
				}

				// get endpoint if in sync to reverse paket, we need to handle some special cases here 
				// and search -2 to +4 from theoretical position
				if (is_sync == 5 && ptr == (p1+paket_size+4))
				{
					ptr-=6;
					int tmp_pos=0xFF;
					int tmp_l=-1;
					
					int tmp_step=-2;
					
					while (tmp_step <= 4)
					{
						if ( buf[p1+paket_size+tmp_step] == 0xFD &&
							tmp_l < buf[p1+paket_size+tmp_step-1] &&
							buf[p1+paket_size+tmp_step-1] < paket_size )
						{
							tmp_l=buf[p1+paket_size+tmp_step-1];
							tmp_pos=tmp_step;
						}
						tmp_step+=2;
					}
					if (tmp_pos == 0xFF || tmp_l==-1 )
					{
						eDebug("[RDS/Rass] ancillary sync lost ... try to resync");
						is_sync=0;
						p1=p2=-1;
						sync_try=0;
						paket_size=0;
					}
					else
					{
						p2=p1+paket_size+tmp_pos;
						ptr=ptr+tmp_pos+2;
					}
				}
				
				// process UECP data
				if ( p1 != -1 && p2 != -1 )
				{
					int cnt=buf[--p2];
					while ( cnt-- > 0 )
					{
						unsigned char c = buf[--p2];
					
						if (bsflag == 1) // byte stuffing
						{
							bsflag=2;
							switch (c)
							{
								case 0x00: c=0xFD; break;
								case 0x01: c=0xFE; break;
								case 0x02: c=0xFF; break;
							}
						}

						if (c == 0xFD && bsflag ==0) 
							bsflag=1;
						else
							bsflag=0;
				

						if (bsflag == 0) 
						{

							if ( state == 1 )
								crc=0xFFFF;
							if (( state >= 1 && state < 11 ) || ( state >=26 && state < 36 ))
								crc = crc_ccitt_byte(crc, c);
							
							switch (state)
							{
								case 0:
									if ( c==0xFE ) // Start
										state=1;
									break;
								case 1: // 10bit Site Address + 6bit Encoder Address
								case 2:
								case 3: // Sequence Counter
									++state;
									break;
								case 4:
									leninfo=c;
									++state;
									break;
								case 5:
									switch (c)
									{
										case 0x0A: // Radiotext
											++state;
											break;
										case 0x46: // Radiotext Plus tags
											state=38;
											break;
										case 0xDA: // Rass
											state=26;
											break;
										default: // reset to state 0
											state=0;
									}
								    break;

								// process Radiotext
								case 6: // Data Set Number ... ignore
								case 7: // Program Service Number ... ignore
									++state;
									break;
								case 8: // Message Element Length
									text_len=c;
									if ( !text_len || text_len > 65 || text_len > leninfo-4)
										state=0;
									else
									{
										++state;
										text_len-=2;
										m_ptr=0;
									}
									break;
								case 9: // Radio Text Status bit:
									// 0   = AB-flagcontrol
									// 1-4 = Transmission-Number
									// 5-6 = Buffer-Config
									++state; // ignore ...
									break;
								case 10:
									// TODO build a complete radiotext charcode to UTF8 conversion table for all character > 0x80
									switch (c)
									{
										case 0 ... 0x7f: break;

										case 0x8d: c='ß'; break;
										case 0x91: c='ä'; break;
										case 0xd1: c='Ä'; break;
										case 0x97: c='ö'; break;
										case 0xd7: c='Ö'; break;
										case 0x99: c='ü'; break;
										case 0xd9: c='Ü'; break;
										default: c=' '; break;  // convert all unknown to space
									}
									message[m_ptr++]=c;
									if(text_len)
										--text_len;
									else
										++state;
									break;
								case 11:
									crc16=c<<8;
									++state;
									break;
								case 12:
									crc16|=c;
									message[m_ptr--]=0;
									while(message[m_ptr] == ' ' && m_ptr > 0)
										message[m_ptr--] = 0;
									if ( crc16 == (crc^0xFFFF) )
									{
										rds_text->setText((const char*)message);
										/*emit*/ textReady((const char*)message);
										if (lastmessage[0]==0 && rass_imode_active != 1)
										{
											if (wasVisible & 8)
												wasVisible |= 1;
											else
												rds_text->show();
										}
										memcpy(lastmessage,message,66);
									}
									else
										eDebug("[RDS/Rass] invalid rdstext crc (%s)", message);
									state=0;
									break;

								// process Rass
								case 26: //MEL
										text_len = c;
										text_len2 = c;
										++state;
										text_len-=9;
										text_len2-=9;
										t_ptr=0;
										break;
								case 27: // SID not used atm
										++state;
										break;
								case 28: // SID not used atm

										++state;
										break;
								case 29: // PNR packet number
										part=c<<16;
										++state;
										break;
								case 30: // PNR packet number
										part|=c<<8;
										++state;
										break;
								case 31: // PNR packet number
										part|=c;
										++state;
										break;
								case 32: // NOP number of packets
										parts=c<<16;
										++state;
										break;
								case 33: // NOP number of packets
										parts|=c<<8;
										++state;
										break;
								case 34: // NOP number of packets
										parts|=c;
										++state;
										break;
								case 35:
										datamessage[t_ptr++]=c;
										if(text_len) 
											--text_len;
										else
											++state;
										break;
								case 36:
										crc16=c<<8;
										++state;
										break;
								case 37:
										crc16|=c;
										//eDebug("[RDS/Rass] CRC read: %04X CRC calculated: %04X",crc16,crc^0xFFFF);
										state=0;
										if ( crc16 == (crc^0xFFFF) ) 
										{
											if (partcnt == -1) 
												partcnt=1;
											if (partcnt == part)
											{
												memcpy(qdar+qdar_pos,datamessage,text_len2+1);
												qdar_pos=qdar_pos+text_len2+1;
												if (partcnt == parts)
												{
													process_qdar(qdar); // decode qdar archive
													qdar_pos=0;
													partcnt=-1;
												}
												else
													++partcnt;
											}
											else
											{
												qdar_pos=0;
												partcnt=-1;
											}
										}
										else
										{
										  eDebug("[RDS/Rass] CRC error, skip Rass-Qdar-Packet");
										  partcnt=-1;
										}
										state=0;
										break;

								// process RT plus tags ... 
								case 38: // Message Element Length
										text_len=c;	
										++state;
										break;
								case 39: // Application ID 
								case 40: // always 0x4BD7 so we ignore it ;)
								case 41: // Applicationgroup Typecode/PTY ... ignore
										++state;
										break;
								case 42:
										rtp_buf[0]=c;
										++state;
										break;
								case 43:
										rtp_buf[1]=c;
										++state;
										break;
								case 44:
										rtp_buf[2]=c;
										++state;
										break;
								case 45:
										rtp_buf[3]=c;
										++state;
										break;
								case 46: // bit 10#4 = Item Togglebit
										 // bit 10#3 = Item Runningbit
										 // Tag1: bit 10#2..11#5 = Contenttype, 11#4..12#7 = Startmarker, 12#6..12#1 = Length
										rtp_buf[4]=c;
										if (lastmessage[0] == 0) // no rds message till now ? quit ...
											break;
										int rtp_typ[2],rtp_start[2],rtp_len[2];
										rtp_typ[0]   = (0x38 & rtp_buf[0]<<3) | rtp_buf[1]>>5;
										rtp_start[0] = (0x3e & rtp_buf[1]<<1) | rtp_buf[2]>>7;
										rtp_len[0]   = 0x3f & rtp_buf[2]>>1;
										// Tag2: bit 12#0..13#3 = Contenttype, 13#2..14#5 = Startmarker, 14#4..14#0 = Length(5bit)
										rtp_typ[1]   = (0x20 & rtp_buf[2]<<5) | rtp_buf[3]>>3;
										rtp_start[1] = (0x38 & rtp_buf[3]<<3) | rtp_buf[4]>>5;
										rtp_len[1]   = 0x1f & rtp_buf[4];
										
										unsigned char rtplus_osd_tmp[64];
										
										if (rtp_start[0] < 66 && (rtp_len[0]+rtp_start[0]) < 66)
										{
											memcpy(rtp_item[rtp_typ[0]],lastmessage+rtp_start[0],rtp_len[0]+1);
											rtp_item[rtp_typ[0]][rtp_len[0]+1]=0;
										}
										
										if (rtp_typ[0] != rtp_typ[1])
										{
											if (rtp_start[1] < 66 && (rtp_len[1]+rtp_start[1]) < 66)
											{
												memcpy(rtp_item[rtp_typ[1]],lastmessage+rtp_start[1],rtp_len[1]+1);
												rtp_item[rtp_typ[1]][rtp_len[1]+1]=0;
											}
										}

										// main RTPlus item_types used by the radio stations:
										// 1 title
										// 4 artist
										// 24 info.date_time
										// 31 stationname
										// 32 program.now
										// 39 homepage
										// 41 phone.hotline
										// 46 email.hotline
										// todo: make a window to display all saved items ...
										
										//create RTPlus OSD for title/artist
										rtplus_osd[0]=0;
										
										if ( rtp_item[4][0] != 0 )//artist
											sprintf((char*)rtplus_osd_tmp," (%s)",rtp_item[4]);
										
										if ( rtp_item[1][0] != 0 )//title
											sprintf((char*)rtplus_osd,"%s%s",rtp_item[1],rtplus_osd_tmp);
										
										if ( rtplus_osd[0] != 0 )
										{
											rtp_text->setText((const char*)rtplus_osd);
											/*emit*/ textReady((const char*)rtplus_osd);
											if (rass_imode_active != 1)
											{
												if (wasVisible & 8)
													wasVisible |= 2;
												else
													rtp_text->show();
											}
										}
										
										state=0;
										break;
							}
						}
					}
					p1=ptr;
					p2=-1;
				}
				++ptr;
			}
			if (p1 != -1 && (263-(p1)) != 263)
			{
				bytesread=ptr=263-(p1);
				memcpy(buf, buf+p1, ptr);
				p1=0;
			}
			else
				bytesread=0;ptr=0;
		}
	}
}

// Rass InteractiveMode
void RDSTextDecoder::rass_interactive()
{
	RassInteractivemode rass;
	m_interactive=&rass;

	rass_imode_active=1;

	if (!qdarmvi_show)
		eStreamWatchdog::getInstance()->reloadSettings(3); // set mpeg decoder to 16:9
	Decoder::displayIFrameFromFile("/tmp/Rass0000.mvi");
	
	rass_logo.hide();
	rass_logo.move( ePoint( 50, 180 ) );
	rass_logo.resize( eSize( 100, 21 ) );
	rass_logo.show();
	
	rass.show();
	rass.exec();
	m_interactive=0;
	rass.hide();
	
	int x = eSkin::getActive()->queryValue("rass_logo.pos.x", 0);
	int y = eSkin::getActive()->queryValue("rass_logo.pos.y", 0);
	rass_logo.hide();
	rass_logo.move( ePoint( x, y ) );
	rass_logo.resize( eSize( 50, 21 ) );
	rass_logo.show();
	
	rass_imode_active=0;
	Decoder::displayIFrameFromFile("/tmp/RassLast.mvi");
}

void RDSTextDecoder::globalFocusHasChanged(const eWidget* newFocus)
{
	if ( !sn ) // not running
		return;
	if ( newFocus )
	{
		wasVisible |= 8;
		if (rds_text->isVisible())
		{
			wasVisible |= 1;
			rds_text->hide();
		}
		if (rtp_text->isVisible())
		{
			wasVisible |= 2;
			rtp_text->hide();
		}
		if (!rass_imode_active && rass_logo.isVisible())
		{
			wasVisible |= 4;
			rass_logo.hide();
		}
	}

	else
	{
		wasVisible &= ~8;
		if (wasVisible & 1)
		{
			wasVisible &= ~1;
			rds_text->show();
		}
		if (wasVisible & 2)
		{
			wasVisible &= ~2;
			rtp_text->show();
		}
		if (wasVisible & 4)
		{
			wasVisible &= ~4;
			rass_logo.show();
		}
	}
}

RassInteractivemode::RassInteractivemode()
	:eWidget(0,1), active_slide(0), active_slide_sub(-1)
{
	init_RassInteractivemode();
}
void RassInteractivemode::init_RassInteractivemode()
{
	gFont rds_font = eSkin::getActive()->queryFont("rass");
	
	cmove(ePoint(50,201)); cresize(eSize(100,254));
	setProperty("backgroundColor","rass_bg");
	setProperty("foregroundColor","rass_fg");
	setFont(rds_font);	
	
	int i=0;
	char c[3];
	
	while (i < 10)
	{
		sprintf(c,"%01d",i);
		rass_page[i]=new eLabel(this);
		rass_page[i]->setText(c);
		rass_page[i]->move(ePoint(5, (25*i)+4));
		rass_page[i]->resize(eSize(70, 25));
		i++;
	}
	
	sprintf(c,"%01d >",0); //set marker
	rass_page[0]->setText(c);
	
	rass_pm1 = eSkin::getActive()->queryImage("rass_page1");
	rass_pm2 = eSkin::getActive()->queryImage("rass_page2");
	rass_pm3 = eSkin::getActive()->queryImage("rass_page3");
	rass_pm4 = eSkin::getActive()->queryImage("rass_page4");
	
	rass_page_sub[0]=new eLabel(this);
	rass_page_sub[0]->setText("Index");
	rass_page_sub[0]->move( ePoint(36, 4));
	rass_page_sub[0]->resize( eSize( 50, 25 ) );
	
	i=1;
	while (i < 10)
	{
		rass_page_sub[i]=new eLabel(this);
		update_avail(i);
		rass_page_sub[i]->move( ePoint(35, (25*i)+6));
		rass_page_sub[i]->resize( eSize( 36, 20 ) );
		rass_page_sub[i]->pixmap_position = ePoint(0,0);
		rass_page_sub[i]->setBlitFlags( BF_ALPHATEST );
		i++;
	}
	
	addActionMap(&i_shortcutActions->map);
	addActionMap(&i_cursorActions->map);
}

RassInteractivemode::~RassInteractivemode()
{
	int i=0;
	while (i < 10)
	{
		delete rass_page[i];
		if (i)
			delete rass_page_sub[i];
		++i;
	}

}

void RassInteractivemode::sel_entry(int val)
{
	char fname[50];
	int file[4];
	int i=1;
	int tmp=0;
	
	while(i < 10) // update availability
		update_avail(i++);
	
	i=0;
	
	if (val == 0xFF) // up
	{
		val=active_slide;
		while (tmp == 0)
		{
			if (--val < 0)
				val=9;
			tmp=check_avail(val);
		}
	}
	
	if (val == 0xFE) // down
	{
		val=active_slide;
		while (tmp == 0)
		{
			if (++val > 9)
				val=0;
			tmp=check_avail(val);
		}
	}
	
	if (val == 0xFD) // left
	{
		val=active_slide;
		i=3;
	}
	
	if (val == 0xFC) // right
		val=active_slide;
	
	sprintf(fname,"/tmp/Rass%04d.mvi",val*1000);
	file[0]=check_file(fname);
	sprintf(fname,"/tmp/Rass%04d.mvi",val*1100);
	file[1]=check_file(fname);
	sprintf(fname,"/tmp/Rass%04d.mvi",val*1110);
	file[2]=check_file(fname);
	sprintf(fname,"/tmp/Rass%04d.mvi",val*1111);
	file[3]=check_file(fname);

	if ((file[0] + file[1] + file[2] + file[3]) == 0)
		return;
	
	if (active_slide != val) 
	{
		active_slide_sub=-1;
		
		char c[3];
		sprintf(c,"%01d",active_slide);
		rass_page[active_slide]->setText(c);
		sprintf(c,"%01d >",val);
		rass_page[val]->setText(c);
	}
	
	tmp=active_slide_sub;
	active_slide_sub=-1;
	
	if (i == 0) //next subslide (right)
	{
		while(i < 4)
		{
			if (active_slide_sub == -1 && file[i] == 1)
				active_slide_sub=i;
			if (file[i] == 1 && i > tmp)
			{
				active_slide_sub=i;
				break;
			}
			i++;
		}
	}
	else //last subslide (left)
	{
		while(i >= 0)
		{
			if (active_slide_sub == -1 && file[i] == 1)
				active_slide_sub=i;
			if (file[i] == 1 && i < tmp)
			{
				active_slide_sub=i;
				break;
			}
			i--;
		}
	}
	active_slide=val;
	
	switch(active_slide_sub)
	{
		case 0:
			val=active_slide*1000;
			break;
		case 1:
			val=active_slide*1100;
			break;
		case 2:
			val=active_slide*1110;
			break;
		case 3:
			val=active_slide*1111;
			break;
		default:
			return;
	}
	
	sprintf(fname,"/tmp/Rass%04d.mvi",val);
	Decoder::displayIFrameFromFile(fname);
}

void RassInteractivemode::update_avail(int i)
{
	if (i < 1 || i > 9)
		return;
	switch(check_avail(i))
	{
		case 1:
			rass_page_sub[i]->setPixmap(rass_pm1);
			break;
		case 2:
			rass_page_sub[i]->setPixmap(rass_pm2);
			break;
		case 3:
			rass_page_sub[i]->setPixmap(rass_pm3);
			break;
		case 4:
			rass_page_sub[i]->setPixmap(rass_pm4);
			break;
		default:
			rass_page_sub[i]->setPixmap(0);
	}
}

int RassInteractivemode::check_file(char* fname)
{
	FILE * f= fopen (fname,"r");
	if (!f) 
		return 0;
	else 
	{
		fclose(f);
		return 1;
	}
}

int RassInteractivemode::check_avail(int page)
{
	if (page == 0)
		return 1;
	
	char fname[50];
	int tmp=0;
	
	sprintf(fname,"/tmp/Rass%04d.mvi",page*1000);
	tmp+=check_file(fname);
	sprintf(fname,"/tmp/Rass%04d.mvi",page*1100);
	tmp+=check_file(fname);
	sprintf(fname,"/tmp/Rass%04d.mvi",page*1110);
	tmp+=check_file(fname);
	sprintf(fname,"/tmp/Rass%04d.mvi",page*1111);
	tmp+=check_file(fname);
	
	return tmp;
}

int RassInteractivemode::eventHandler( const eWidgetEvent &e )
{
	switch( e.type )
	{
	case eWidgetEvent::evtAction:
		if ( e.action == &i_shortcutActions->number0)
			sel_entry(0);
		else if ( e.action == &i_shortcutActions->number1)
			sel_entry(1);
		else if ( e.action == &i_shortcutActions->number2)
			sel_entry(2);
		else if ( e.action == &i_shortcutActions->number3)
			sel_entry(3);
		else if ( e.action == &i_shortcutActions->number4)
			sel_entry(4);
		else if ( e.action == &i_shortcutActions->number5)
			sel_entry(5);
		else if ( e.action == &i_shortcutActions->number6)
			sel_entry(6);
		else if ( e.action == &i_shortcutActions->number7)
			sel_entry(7);
		else if ( e.action == &i_shortcutActions->number8)
			sel_entry(8);
		else if ( e.action == &i_shortcutActions->number9)
			sel_entry(9);
		else if ( e.action == &i_cursorActions->up)
			sel_entry(0xFF);
		else if ( e.action == &i_cursorActions->down)
			sel_entry(0xFE);
		else if ( e.action == &i_cursorActions->left)
			sel_entry(0xFD);
		else if ( e.action == &i_cursorActions->right)
			sel_entry(0xFC);
		else if ( e.action == &i_cursorActions->cancel)
			close(0);
		else
			break;
		return 1;
	default:
		break;
	}
	return eWidget::eventHandler(e);
}

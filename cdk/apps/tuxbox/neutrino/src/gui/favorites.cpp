/*
	Neutrino-GUI  -   DBoxII-Project

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <zapit/client/zapitclient.h>

#include <global.h>
#include <neutrino.h>

#include "favorites.h"
#include "gui/channellist.h"

#include "widget/messagebox.h"
#include "widget/hintbox.h"


//
// -- Add current channel to Favorites-Bouquet
// -- Return Status (bit-Status):A
// --    1 = Bouquet created
// --    2 = Channel added   (if not set, channel already in BQ)
// -- rasc
//

int CFavorites::addChannelToFavorites()

{
	signed int   bouquet_id;
	t_channel_id channel_id;
	char         *fav_bouquetname;
	int          status = 0;


	// no bouquet-List?  do nothing
	if (!bouquetList) return status;

	// -- get Favorites Bouquetname  from Locales
	fav_bouquetname = (char *) g_Locale->getText("favorites.bouquetname").c_str();

	//
	// -- check if Favorite Bouquet exists: if not, create it.
	//
	bouquet_id = g_Zapit->existsBouquet(fav_bouquetname);
	if (bouquet_id == -1) {
		g_Zapit->addBouquet(fav_bouquetname);
	        bouquet_id = g_Zapit->existsBouquet(fav_bouquetname);
		status |= 1;
	}


	channel_id = g_Zapit->getCurrentServiceID();

	if ( ! g_Zapit->existsChannelInBouquet(bouquet_id, channel_id) ) {
		g_Zapit->addChannelToBouquet(bouquet_id, channel_id);
		status |= 2;
	}


	// -- tell zapit to save Boquets and reinit (if changed)
	if (status) {
		g_Zapit->saveBouquets();        // better be safe than sorry (save first, commit later)
		g_Zapit->commitBouquetChange();
	}

	return status;
}


//
// -- Menue Handler Interface
// -- to fit the MenueClasses from McClean
// -- Add current channel to Favorites and display user messagebox
//

int CFavorites::exec(CMenuTarget* parent, string)
{
	int    res = menu_return::RETURN_EXIT_ALL;
	int    status;
	string str;


	if (parent)
	{
		parent->hide();
	}

	if (!bouquetList) {
		ShowMsg ( "favorites.bouquetname",
			g_Locale->getText("favorites.nobouquets"),
			CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw" );
		return res;
	}


	CHintBox* hintBox= new CHintBox( "favorites.bouquetname", g_Locale->getText("favorites.addchannel"), "info.raw", 380 );
	hintBox->paint();

	status = addChannelToFavorites();

	hintBox->hide();
	delete hintBox;

	// -- Display result

	str = "";
	if (status & 1)  str += g_Locale->getText("favorites.bqcreated");
	if (status & 2)  str += g_Locale->getText("favorites.chadded");
	else             str += g_Locale->getText("favorites.chalreadyinbq");

	if (status) str +=  g_Locale->getText("favorites.finalhint");

	ShowMsg ( "favorites.bouquetname", str, CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw" );

//	if (status) {
//		g_RCInput->postMsg( NeutrinoMessages::EVT_BOUQUETSCHANGED, 0 );
//	}

	return res;
}

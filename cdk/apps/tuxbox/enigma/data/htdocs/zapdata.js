var channels = new Array();
var channelRefs = new Array();
var bouquets = new Array(
		#BOUQUETS#
		);
var bouquetRefs = new Array(
		#BOUQUETREFS#
		);
var currentBouquet = #CURRENTBOUQUET#;
var currentChannel = #CURRENTCHANNEL#;
var autoBouquetChange = #AUTOBOUQUETCHANGE#;
var zapMode = #ZAPMODE#;
var zapSubMode = #ZAPSUBMODE#;
			
function init()
{	
	#CHANNELS#
	#CHANNELREFS#

	loadBouquets(currentBouquet);
	loadChannels(currentBouquet, currentChannel);
}
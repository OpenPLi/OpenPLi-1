function headerUpdateVolumeBar(volume, mute)
{
	for (var i = 9; i <= 63; i += 6)
	{
		var vol = 0;
		if (mute == 0)
			vol = volume;
		if (i <= vol)
			getElem("id", "imgVol" + i, null).src = "led_on.gif";
		else
			getElem("id", "imgVol" + i, null).src = "led_off.gif";
	}
	if (mute == 0)
		getElem("id", "mute", null).src = "speak_on.gif";
	else
		getElem("id", "mute", null).src = "speak_off.gif";
}

function headerUpdateRecording(recording, recordChan)
{
	if (recording == 1)
	{
		getElem("id", "recording", null).src = "blinking_red.gif";
		getElem("id", "recordChan", null).firstChild.nodeValue = recordChan;
	}
	else
	{
		getElem("id", "recording", null).src = "trans.gif";
		getElem("id", "recordChan", null).firstChild.nodeValue = "";
	}
}

function headerUpdateChannelStatusBar(dolby, crypt, format)
{
	
	if (crypt == 1)
		getElem("id", "imgCrypt", null).src = "crypt_on.gif";
	else
		getElem("id", "imgCrypt", null).src = "crypt_off.gif";
	
}

function headerUpdateStatusBar(vpid, apid, ip, lock, upTime, agc, snr, ber, agcrel, snrrel, berrel, satSyncStatus, satLockStatus)
{
//	document.getElementById("pids").innerHTML = 'vpid:&nbsp;<a class="minitext" href="javascript:document.location=\'javascript:vlc()\';">' + vpid + '</a>&nbsp; \

	document.getElementById("pids").innerHTML = 'vpid:&nbsp;<a class="minitext" href="javascript:vlc();">' + vpid + '</a>&nbsp; \
apid:&nbsp;<a class="minitext" href="javascript:document.location=\'audio.pls\';">' + apid + '</a>';
	if (parseInt(snr) <= 20) {
		snr = "" + snr + "dB";
	}
	document.getElementById("snrval").innerHTML = snr;
	document.getElementById("snrbar").style.width = "" + snrrel/2 + "px";
	document.getElementById("agcval").innerHTML = agc;
	document.getElementById("agcbar").style.width = "" + agcrel/2 + "px";
	document.getElementById("berval").innerHTML = ber;
	document.getElementById("berbar").style.width = "" + berrel/2 + "px";
	document.getElementById("satsync").src = "lock_" + satSyncStatus + ".gif";
	document.getElementById("satlock").src = "lock_" + satLockStatus + ".gif";
}

function headerUpdateEPGData(serviceName, nowT, nowD, nowSt, nextT, nextD, nextSt, agc)
{
	getElem("id", "servicename", null).firstChild.nodeValue = serviceName;
	getElem("id", "nowt", null).firstChild.nodeValue = nowT;
	getElem("id", "nowd", null).firstChild.nodeValue = nowD;
	if (!MS)
		nowSt = nowSt.substr(0, 40);
	getElem("id", "nowst", null).firstChild.nodeValue = nowSt;
	getElem("id", "nextt", null).firstChild.nodeValue = nextT;
	getElem("id", "nextd", null).firstChild.nodeValue = nextD;
	if (!MS)
		nextSt = nextSt.substr(0, 40);
	getElem("id", "nextst", null).firstChild.nodeValue = nextSt;
	
}

function topnavi(command)
{
	parent.body.location = "body" + command;
	if (command != '?mode=timers') {
		parent.leftnavi.location = "leftnavi" + command;
	}
}

function setTitle(title)
{
	getElem("id", "title", null).firstChild.nodeValue = title;
}

function init()
{
	data.location.reload();
	setTimeout("init()", 10000);
}

function updateVideoBar(videoPosition, videoTime, diskGB, diskH)
{
	getElem("id", "videoTime", null).firstChild.nodeValue = videoTime;
	getElem("id", "diskgb", null).firstChild.nodeValue = diskGB;
	getElem("id", "diskh", null).firstChild.nodeValue = diskH;
}

function video(key)
{
	if (body.zapMode)
	{
		if (body.zapMode == 3)
		{
			var selChannel = body.document.channelselector.channel.selectedIndex;
			var channel = "";
			if (selChannel >= 0)
			{
				currentChannel = selChannel;
				channel = body.document.channelselector.channel.options[selChannel].value;
				document.location = "/cgi-bin/videocontrol?command=" + key + "&sref=" + channel + "&curChannel=" + currentChannel;
			}
		}
		else
			document.location = "/cgi-bin/videocontrol?command=" + key;
	}
	else
		document.location = "/cgi-bin/videocontrol?command=" + key;
}

var tmpBrowser = navigator.appName;
var IE7test;
var vidString;
var browser;
var hideMenu;
if (tmpBrowser.indexOf('Microsoft') != -1)
{
	browser = 'MSIE';
	vidString = 'MSIE';
	hideMenu = 1;
	IE7test = navigator.appVersion.split("MSIE")
	if (parseFloat(IE7test[1]) == 7) 
	{
		browser = 'MSIE7';
		vidString = 'MSIE';
		hideMenu = 0;
	}
} else if ((tmpBrowser == 'Netscape') && (navigator.userAgent.indexOf('Gecko') != -1)) {
	browser = 'Firefox';
	vidString = 'Firefox';
	hideMenu = 0;
} else {
	browser = 'Small'
	vidString = 'Firefox';
	hideMenu = 1;
}

function webxCheck() {
	document.writeln("<a href=javascript:webxtv('" + vidString + "')>Web-X-TV</a>");
}

function webxtv(tmp)
{
	NewWindow("/webxtv?browser=" + tmp, "tv", "780", "735", "no", "0");
}

function teletext()
{
	NewWindow("/teletext", "teletext", "480", "560", "no", "0");
}

function menuAct(inst) {
	if (hideMenu == 1) {document.getElementById('body').style.visibility = 'hidden';}
	document.getElementById('img' + inst + '').src = 'arrow_down.png';
	document.getElementById('menu' + inst + '').style.visibility = 'visible';
	document.getElementById('link' + inst + '').style.color = '#f2f3f5';
	
	for (i = 1; i < 5; i++) {
		if (i != inst) {
			document.getElementById('img' + i + '').src = 'arrow_up.png';
			document.getElementById('menu' + i + '').style.visibility = 'hidden';
			document.getElementById('link' + i + '').style.color = '#c1cbdf';
		} 
	}
}

function inAct(inst) {
	if (hideMenu == 1) {document.getElementById('body').style.visibility = 'visible';}
	document.getElementById('img' + inst + '').src = 'arrow_up.png';
	document.getElementById('menu' + inst + '').style.visibility = 'hidden';
	document.getElementById('link' + inst + '').style.color = '#c1cbdf';
}

function inActAll() {
	for (i = 1; i < 5; i++) {
		document.getElementById('img' + i + '').src = 'arrow_up.png';
		document.getElementById('menu' + i + '').style.visibility = 'hidden';
		document.getElementById('link' + i + '').style.color = '#c1cbdf';
	}
	if (hideMenu == 1) {document.getElementById('body').style.visibility = 'visible';}
}

function testConn(url) 
{
if (browser == 'Firefox') document.body.style.cursor = 'wait' ;
var img = new Image(); 
img.onload = img_onLoad;
img.onerror = img_onError;
img.src = "http://downloads.pli-images.org/jade/xml/all/check.gif";
} 

function img_onLoad()
{
	if (document.getElementById('newsticker').innerHTML == '') {
		document.getElementById('newsticker').innerHTML = '<iframe name="ticker" src="http://downloads.pli-images.org/jade/xml/all/ticker.php" width="780px" height="22" frameborder="0" framemargin="0" scrolling="no"></iframe>';
	} else {
	document.getElementById('newsticker').innerHTML = '';
	}
	if (browser == 'Firefox') document.body.style.cursor = 'default';
}

function img_onError() {
	var iChk = window.confirm("No internet connection detected, retry?");
	if (iChk) {
		testConn();
	} else {
		if (browser == 'Firefox') document.body.style.cursor = 'default';
	}
}

setTimeout('document.getElementById("imgTop").src = "top.jpg"',15000);

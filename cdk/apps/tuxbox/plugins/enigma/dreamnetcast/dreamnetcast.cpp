/*
 *  DreamNetCast
 *
 *  Author: xphile
 *
 *  This source file contains the definitions needed to implement
 *  the DreamCast plugin
 *
 *
 *	30/05/2004 - xphile - Initial release
 *	20/07/2004 - xphile - Version 1.0
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include <plugin.h>
#include <xmltree.h>

#include <linux/soundcard.h>

#include <lib/base/buffer.h>
#include <lib/base/eerror.h>
#include <lib/codecs/codec.h>
#include <lib/codecs/codecmp3.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvb.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/emessage.h>
#include <lib/gui/ewidget.h>
#include <lib/gui/enumber.h>
#include <lib/system/httpd.h>
#include <lib/system/econfig.h>

#include "dreamnetcast.h"
#include "filemgr.h"


void showBackgroundPic()
{
	FILE *f = fopen(CONFIGDIR "/enigma/pictures/radio.mvi", "r");
	if ( f )
	{
		fclose(f);
		Decoder::displayIFrameFromFile(CONFIGDIR "/enigma/pictures/radio.mvi" );
	}
	else
		Decoder::displayIFrameFromFile(DATADIR "/enigma/pictures/radio.mvi" );
}

/*
 * Sets the widgets font size
 */
void setWidgetFont(eWidget *wdgt)
{
	gFont font = wdgt->getFont();
	int size = font.pointSize;

	if (font.family == "Blue.Regular")
		size = 24;
	else if (font.family == "Nimbus")
		size = 16;

	wdgt->setFont(gFont(font.family, size));

//	eDebug(font.family.c_str());
}

scHTTPStream::scHTTPStream(eHTTPConnection *c, eIOBuffer &buffer)
	: eHTTPDataSource(c), buffer(buffer)
{
	eDebug("HTTP stream sink created!");
	metadatainterval=metadataleft=bytes=0;
	if (c->remote_header.count("icy-metaint"))
		metadatainterval = atoi(c->remote_header["icy-metaint"].c_str());
	if (c->remote_header.count("icy-name"))
		icyName = c->remote_header["icy-name"];
	if (c->remote_header.count("icy-genre"))
		icyGenre = c->remote_header["icy-genre"];
}

scHTTPStream::~scHTTPStream()
{
	eDebug("HTTP stream sink deleted!");
}

void scHTTPStream::processMetaData()
{
	metadata[metadatapointer]=0;
	metaDataUpdated((const char*)metadata);
//	eDebug("processing metadata! %s", metadata);

	metadatapointer=0;
}

void scHTTPStream::haveData(void *vdata, int len)
{
	__u8 *data=(__u8*)vdata;

	while (len)
	{
		int valid=len;
		if (!metadataleft)
		{
			//
			// not in metadata mode.. process mp3 data (stream to input buffer)
			// are we just at the beginning of metadata? (pointer)
			if (metadatainterval && (metadatainterval == bytes))
			{
				// enable metadata mode
				metadataleft=*data++*16;
				metadatapointer=0;
				len--;
				bytes=0;
				continue;
			}
			else if (metadatainterval && (metadatainterval < bytes))
				eFatal("metadatainterval < bytes");

			// otherwise there's really data.
			if (metadatainterval)
			{
				// is metadata in our buffer?
				if ((valid + bytes) > metadatainterval)
					valid=metadatainterval-bytes;
			}

			buffer.write(data, valid);
			data+=valid;
			len-=valid;
			bytes+=valid;
		}
		else
		{
			// metadata ... process it.
			int meta=len;
			if (meta > metadataleft)
				meta=metadataleft;

			memcpy(metadata+metadatapointer, data, meta);
			metadatapointer+=meta;
			data+=meta;
			len-=meta;
			metadataleft-=meta;

			if (!metadataleft)
				processMetaData();
		}
	}

	dataAvailable();
}

StreamDecoder::StreamDecoder(const char *filename, DreamCastDialog *theDialog, int minBufferSize)
	:outputbr(0), dialog(theDialog), input(INPUT_SIZE), output(OUTPUT_SIZE)
{
	state = stateInit;

	http = 0;

	http = eHTTPConnection::doRequest(filename, eApp, &error);

	if (!http)
	{
		streamingDone(error);
	}
	else
	{
		CONNECT(http->transferDone, StreamDecoder::streamingDone);
		CONNECT(http->createDataSource, StreamDecoder::createStreamSink);
		http->local_header["User-Agent"] = "DreamNetCast-mp3/1.0.0";
		http->local_header["Icy-MetaData"] = "1"; // enable ICY metadata
		http->start();
		http_status = _("Connecting...");
		dialog->setStatus(http_status);
	}

	pcmsettings.reconfigure = 1;

	dspfd=::open("/dev/sound/dsp", O_WRONLY|O_NONBLOCK);

	if (dspfd < 0)
	{
		eDebug("output failed! (%m)");
		error = errno;
		state = stateError;
	}

	if (dspfd >= 0)
	{
		outputsn = new eSocketNotifier(eApp, dspfd, eSocketNotifier::Write, 0);
		CONNECT(outputsn->activated, StreamDecoder::outputReady);
	}
	else
		outputsn = 0;

	audiodecoder = new eAudioDecoderMP3(input, output);
	maxOutputBufferSize = OUTPUT_SIZE;
	minOutputBufferSize = minBufferSize;
}

StreamDecoder::~StreamDecoder()
{
	dspSync();

	delete outputsn;

	if (dspfd >= 0)
		close(dspfd);

	if (http)
		delete http;

	delete audiodecoder;

	eDebug("StreamDecoder::~StreamDecoder - start/end");
}

void StreamDecoder::metaDataUpdated(eString meta)
{
	eLocker locker(poslock);
	if (meta.left(6) == "Stream")
		while (!meta.empty())
		{
			unsigned int eq=meta.find('=');
			if (eq == eString::npos)
					break;
			eString left=meta.left(eq);
			meta=meta.mid(eq+1); // skip until =
			eq=meta.find(';');
			if (eq == eString::npos)
				break;
			eString right=meta.left(eq);
			meta=meta.mid(eq+1);
			if (left=="StreamTitle")
				streamTitle=right;
			else if (left == "StreamUrl")
				streamUrl=right;
			else
				eDebug("unknown tag: %s = %s", left.c_str(), right.c_str());
		}
	else
		streamTitle=meta;

	dialog->setIcyName(stream->icyName);
	dialog->setIcyGenre(stream->icyGenre);
	dialog->setStreamTitle(streamTitle);
}

void StreamDecoder::streamingDone(int err)
{
	if (err || !http || http->code != 200)
	{
		eLocker locker(poslock);
		if (err)
		{
			switch (err)
			{
			case -2:
				http_status="Can't resolve hostname!";
				break;
			case -3:
				http_status="Can't connect!";
				break;
			default:
				http_status.sprintf("unknown error %d", err);
			}
		}
		else if (http && (http->code!=200))
			http_status.sprintf("error: %d (%s)", http->code, http->code_descr.c_str());
		else
			http_status="unknown";

		dialog->setError(http_status.c_str());
	}
	else
	{
		state = stateFileEnd;

		if (outputsn)
			outputsn->start();

		eDebug("streaming ended");
	}

	http = 0;
}

void StreamDecoder::decodeMoreHTTP()
{
	checkFlow(0);
}

void StreamDecoder::outputReady(int what)
{
	(void)what;

	if ( ( pcmsettings.reconfigure
			|| (pcmsettings.samplerate != audiodecoder->pcmsettings.samplerate)
			|| (pcmsettings.channels != audiodecoder->pcmsettings.channels)))
	{
		pcmsettings = audiodecoder->pcmsettings;

		outputbr = pcmsettings.samplerate*pcmsettings.channels*16;
		if (::ioctl(dspfd, SNDCTL_DSP_SPEED, &pcmsettings.samplerate) < 0)
			eDebug("SNDCTL_DSP_SPEED failed (%m)");
		if (::ioctl(dspfd, SNDCTL_DSP_CHANNELS, &pcmsettings.channels) < 0)
			eDebug("SNDCTL_DSP_CHANNELS failed (%m)");
		if (::ioctl(dspfd, SNDCTL_DSP_SETFMT, &pcmsettings.format) < 0)
			eDebug("SNDCTL_DSP_SETFMT failed (%m)");
	}

	output.tofile(dspfd, 65536);
	checkFlow(0);
}

void StreamDecoder::checkFlow(int last)
{
	int i = input.size(), o;
	o = output.size();

//	eDebug("Output buffer size = %d", o);

	// states:
	// buffering  -> output is stopped (since queue is empty)
	// playing    -> input queue (almost) empty, output queue filled
	// bufferFull -> input queue full, reading disabled, output enabled
	if (!o)
	{
		if (state == stateFileEnd)
		{
			if (outputsn)
				outputsn->stop();
			eDebug("ok, everything played..");
			return;
		}
		else if (state != stateBuffering)
		{
			dialog->setStatus(_("Buffering..."));
			state = stateBuffering;
			if (outputsn)
				outputsn->stop();
		}
	}

	if ( o > maxOutputBufferSize)
	{
		if (state != stateBufferFull)
		{
//			eDebug("stateBufferFull");
			if (http)
				http->disableRead();
			state = stateBufferFull;
		}
	}

	if ( o < maxOutputBufferSize)
	{
		int samples=0;

		if (last || (i >= audiodecoder->getMinimumFramelength()))
		{
			Signal1<void, unsigned int> callback;
			CONNECT(callback, StreamDecoder::newAudioStreamIdFound);
			samples = audiodecoder->decodeMore(last, 16384, &callback);
		}

		if (samples < 0)
		{
			state = stateFileEnd;
			eDebug("Stream ended");
		}

		if ((o + samples) < maxOutputBufferSize)
		{
			if (state == stateBufferFull)
			{
//				eDebug("stateBufferFull -> statePlaying");
				state = statePlaying;
			}

			if (http)
				http->enableRead();
		}
	}

	if ((state == stateBuffering) && (o > minOutputBufferSize))
	{
		dialog->setStatus(_("Playing..."));
		state = statePlaying;
		if (outputsn)
			outputsn->start();
		else
			dialog->setStatus(_("Error /dev/sound/dsp not found..."));
	}
}

void StreamDecoder::dspSync()
{
	eDebug("dspSync");
	if (::ioctl(dspfd, SNDCTL_DSP_RESET) < 0)
		eDebug("SNDCTL_DSP_RESET failed (%m)");
}

void StreamDecoder::newAudioStreamIdFound(unsigned int id)
{
	eDebug("StreamDecoder::newAudioStreamIdFound(%02x)", id);
}

eHTTPDataSource *StreamDecoder::createStreamSink(eHTTPConnection *conn)
{
	eDebug("creating the stream sink");

	stream = new scHTTPStream(conn, input);
	CONNECT(stream->dataAvailable, StreamDecoder::decodeMoreHTTP);
	CONNECT(stream->metaDataUpdated, StreamDecoder::metaDataUpdated);
	http_status = _("playing...");

	eDebug(http_status.c_str());
	return stream;
}

/*
 * DreamCastDialog:
 */

/*
 * Constructor
 */
DreamCastDialog::DreamCastDialog()
	: mp3Decoder(0), directory(0)
{
	eDebug("DreamCastDialog::DreamCastDialog - start");

	cmove(ePoint(100, 110));
	cresize(eSize(510, 280));
	setText(_("DreamNetCast v1.0..."));

	dummy = new eListBox<eListBoxEntryText>(this);
	dummy->move(ePoint(1, 1));
	dummy->resize(eSize(0, 0));
	dummy->loadDeco();
	dummy->hide();

	cbx_Station = new eComboBox(this, 4);
	cbx_Station->move(ePoint(10, 10));
	cbx_Station->resize(eSize(clientrect.width() - 20, 30));
	cbx_Station->loadDeco();
	setWidgetFont(cbx_Station);

	lb_IcyNameLbl = new eLabel(this);
	lb_IcyNameLbl->move(ePoint(10, 80));
	lb_IcyNameLbl->resize(eSize(110, 35));
	lb_IcyNameLbl->setText(_("Stream Title:"));
	lb_IcyNameLbl->loadDeco();
	setWidgetFont(lb_IcyNameLbl);

	lb_IcyName = new eLabel(this);
	lb_IcyName->move(ePoint(130, 80));
	lb_IcyName->resize(eSize(370, 35));
	lb_IcyName->setText(_("None"));
	lb_IcyName->loadDeco();
	setWidgetFont(lb_IcyName);

	lb_IcyGenreLbl = new eLabel(this);
	lb_IcyGenreLbl->move(ePoint(10, 100));
	lb_IcyGenreLbl->resize(eSize(110, 35));
	lb_IcyGenreLbl->setText(_("Stream Genre:"));
	lb_IcyGenreLbl->loadDeco();
	setWidgetFont(lb_IcyGenreLbl);

	lb_IcyGenre = new eLabel(this);
	lb_IcyGenre->move(ePoint(130, 100));
	lb_IcyGenre->resize(eSize(370, 35));
	lb_IcyGenre->setText(_("None"));
	lb_IcyGenre->loadDeco();
	setWidgetFont(lb_IcyGenre);

	lb_StreamTitleLbl = new eLabel(this);
	lb_StreamTitleLbl->move(ePoint(10, 120));
	lb_StreamTitleLbl->resize(eSize(110, 35));
	lb_StreamTitleLbl->setText(_("Current Song:"));
	lb_StreamTitleLbl->loadDeco();
	setWidgetFont(lb_StreamTitleLbl);

	lb_StreamTitle = new eLabel(this);
	lb_StreamTitle->move(ePoint(130, 120));
	lb_StreamTitle->resize(eSize(370, 35));
	lb_StreamTitle->setText(_("None"));
	lb_StreamTitle->loadDeco();
	setWidgetFont(lb_StreamTitle);

	btnPlay = new eButton(this);
	btnPlay->setText(_("Play"));
	btnPlay->move(ePoint(20, 210));
	btnPlay->resize(eSize(110, 30));
	btnPlay->setShortcut("green");
    btnPlay->setShortcutPixmap("green");
	btnPlay->loadDeco();
	setWidgetFont(btnPlay);
	CONNECT(btnPlay->selected, DreamCastDialog::playStream);

	btnStop = new eButton(this);
	btnStop->setText(_("Stop"));
	btnStop->move(ePoint(140, 210));
	btnStop->resize(eSize(110, 30));
	btnStop->setShortcut("red");
    btnStop->setShortcutPixmap("red");
	btnStop->loadDeco();
	setWidgetFont(btnStop);
	btnStop->hide();
	CONNECT(btnStop->selected, DreamCastDialog::stopStream);

	btnClose = new eButton(this);
	btnClose->setText(_("Close"));
	btnClose->move(ePoint(140, 210));
	btnClose->resize(eSize(110, 30));
	btnClose->setShortcut("red");
    btnClose->setShortcutPixmap("red");
	btnClose->loadDeco();
	setWidgetFont(btnClose);
	CONNECT(btnClose->selected, eWidget::reject);

	btnSettings = new eButton(this);
	btnSettings->setText(_("Settings"));
	btnSettings->move(ePoint(260, 210));
	btnSettings->resize(eSize(110, 30));
	btnSettings->setShortcut("yellow");
    btnSettings->setShortcutPixmap("yellow");
	btnSettings->loadDeco();
	setWidgetFont(btnSettings);
	CONNECT(btnSettings->selected, DreamCastDialog::updateSettings);

	btnMinimise = new eButton(this);
	btnMinimise->setText(_("Minimise"));
	btnMinimise->move(ePoint(260, 210));
	btnMinimise->resize(eSize(110, 30));
	btnMinimise->setShortcut("yellow");
    btnMinimise->setShortcutPixmap("yellow");
	btnMinimise->loadDeco();
	setWidgetFont(btnMinimise);
	btnMinimise->hide();
	CONNECT(btnMinimise->selected, DreamCastDialog::minimise);

	btnAbout = new eButton(this);
	btnAbout->setText(_("About"));
	btnAbout->move(ePoint(380, 210));
	btnAbout->resize(eSize(110, 30));
	btnAbout->setShortcut("blue");
    btnAbout->setShortcutPixmap("blue");
	btnAbout->loadDeco();
	setWidgetFont(btnAbout);
	CONNECT(btnAbout->selected, DreamCastDialog::about);

	sbStatus = new eStatusBar(this);
	sbStatus->setFlags(eStatusBar::flagOwnerDraw);
	sbStatus->move(ePoint(0, getClientSize().height() - 30));
	sbStatus->resize(eSize(getClientSize().width(), 30));
	sbStatus->loadDeco();

	// Mute the current service
	apid = Decoder::parms.apid;
	Decoder::parms.apid = 0x1ffe;
	Decoder::Set();

	initialise();

	cbx_Station->setCurrent(0);
	setFocus(cbx_Station);

	eDebug("DreamCastDialog::DreamCastDialog - end");
}

/*
 * Destructor
 */
DreamCastDialog::~DreamCastDialog()
{
	if (directory)
		delete directory;

	if (mp3Decoder)
		delete mp3Decoder;

	// Unmute the current service
	Decoder::parms.apid = apid;
	Decoder::Set();

	eDebug("DreamCastDialog::~DreamCastDialog - start/end");
}

/*
 * Sets the status text in the status bar
 */
void DreamCastDialog::setStatus(const eString &string)
{
	eDebug("DreamCastDialog::setStatus - start");
	sbStatus->setText(string);
	eDebug("DreamCastDialog::setStatus - end");
}

/*
 * Error handling routine
 */
void DreamCastDialog::setError(const char *text)
{
	eDebug("DreamCastDialog::setError - start");

	eString errmsg = text;
	setStatus(errmsg);

	if (errmsg.length())
	{
		eMessageBox box(errmsg, _("Error!"), eMessageBox::btOK|eMessageBox::iconError);
		box.show();
		box.exec();
		box.hide();
	}

	eDebug("DreamCastDialog::setError - end");
}

/*
 * Plays a stream
 */
void DreamCastDialog::playStream()
{
	eDebug("DreamCastDialog::playStream - start");

	eListBoxEntryText *selected = cbx_Station->getCurrent();

	if (selected)
	{
		btnPlay->hide();
		btnClose->hide();
		btnSettings->hide();
		btnAbout->hide();
		btnStop->show();
		btnMinimise->show();
//		showBackgroundPic();
		setStatus(_("Locating Server..."));

		if (mp3Decoder)
		{
			delete mp3Decoder;
			mp3Decoder = 0;
		}

		eString command;

		command.sprintf("wget -O /tmp/dncast.tmp %s", selected->getKey());
    eDebug("play stream command: %s", command.c_str());
		int rc = system(command.c_str());

		if (rc == 0)
		{
			//
			// Retry 10 times with a sleep of 1 sec between
			//
			for (int i = 0; i < 10; ++i)
			{
				sleep(1);
				char streamURL[256];

				if (gettag("File1", streamURL) != -1)
				{
					int minBufferSize = 0;
					eConfig::getInstance()->getKey("/dreamnetcast/minbuffersize", minBufferSize);

					minBufferSize = minBufferSize == 0 ? DFLT_MIN_BUFFER : minBufferSize;

					mp3Decoder = new StreamDecoder(streamURL, this, minBufferSize);
					eDebug("streamURL: %c", streamURL);
					break;
				}
			}
		}

		system("rm -f /tmp/dncast.tmp");
	}

	eDebug("DreamCastDialog::playStream - end");
}

/*
 * Stops the stream
 */
void DreamCastDialog::stopStream()
{
	if (mp3Decoder)
	{
		delete mp3Decoder;
		mp3Decoder = 0;
	}

	lb_IcyName->setText(_("None"));
	lb_IcyGenre->setText(_("None"));
	lb_StreamTitle->setText(_("None"));

	setStatus(_("Stopped."));

	btnMinimise->hide();
	btnStop->hide();
	btnAbout->show();
	btnSettings->show();
	btnClose->show();
	btnPlay->show();
}

/*
 * Displays the settings dialog
 */
void DreamCastDialog::updateSettings()
{
	hide();
	DreamCastSettingsDialog dlg;
	dlg.show();
	dlg.exec();
	dlg.hide();

	//
	// Reload XML
	//
	show();
	initialise();
}

/*
 * Displays the about information
 */
void DreamCastDialog::about()
{
	eMessageBox box(ABOUT_STR, _("About"), eMessageBox::btOK|eMessageBox::iconInfo);
	box.cmove(ePoint(160, 130));
	box.show();
	box.exec();
	box.hide();
}

/*
 * Minimises the play window
 */
void DreamCastDialog::minimise()
{
	hide();
	DreamCastMinimisedDialog dlg;
	dlg.show();
	dlg.exec();
	dlg.hide();
	show();
}

void DreamCastDialog::initialise()
{
	char line[100];
	eString data;
	XMLTreeNode *root = 0;

	FILE *fh = fopen(CONFIGDIR "/stations.xml", "r");

	if (fh == 0)
	{
		//
		// File not found
		//
		eString errorstring;

		errorstring.sprintf(CONFIGDIR "/stations.xml not found.");
		setError(errorstring.c_str());
		return;
	}

	while  (fgets(line, 100, fh) != NULL)
		data += line;

	fclose(fh);

	int len = strlen(data.c_str());

	if (directory)
		delete directory;
	directory=new XMLTreeParser("ISO-8859-1");

	if (!directory->Parse(data.c_str(), len, !(data.c_str())))
	{
		//
		// Parse error
		//
		eString errorstring;

		errorstring.sprintf("XML parse error: %s at line %d",
			directory->ErrorString(directory->GetErrorCode()),
			directory->GetCurrentLineNumber());
		setError(errorstring.c_str());
	}
	else
	{
		root = directory->RootNode();

		if (!root)
			setError("XML parse error");
	}

	if (root)
	{
		cbx_Station->clear();
		loadStations(root);
	}
}

void DreamCastDialog::loadStations(XMLTreeNode *root)
{
	for (XMLTreeNode *node1 = root->GetChild(); node1; node1 = node1->GetNext())
	{
		if (!strcmp(node1->GetType(), "station"))
		{
			const char *name = node1->GetAttributeValue("name");
			const char *url = node1->GetAttributeValue("url");

			new eListBoxEntryText(*cbx_Station, name, (void *) url);
		}
		else if (!strcmp(node1->GetType(), "playlist"))
		{
			for (XMLTreeNode *node2 = node1->GetChild(); node2; node2 = node2->GetNext())
			{
				if (!strcmp(node2->GetType(), "entry"))
				{
					const char *name = "";
					const char *url = node2->GetAttributeValue("Playstring");

					for (XMLTreeNode *node3 = node2->GetChild(); node3; node3 = node3->GetNext())
					{
						if (!strcmp(node3->GetType(), "Name"))
							name = node3->GetData();
					}

					new eListBoxEntryText(*cbx_Station, name, (void *) url);
				}
			}
		}
	}
}

void DreamCastDialog::setIcyName(eString &text)
{
	lb_IcyName->setText(eString().sprintf("%s", text.c_str()));
}

void DreamCastDialog::setIcyGenre(eString &text)
{
	lb_IcyGenre->setText(eString().sprintf("%s", text.c_str()));
}

void DreamCastDialog::setStreamTitle(eString &text)
{
	lb_StreamTitle->setText(eString().sprintf("%s", text.c_str()));
}

/*
 * DreamCastSettingsDialog:
 */

/*
 * Constructor
 */
DreamCastSettingsDialog::DreamCastSettingsDialog()
	: limit(25)
{
	eDebug("DreamCastSettingsDialog::DreamCastSettingsDialog - start");

	minBufferSize = 0;
	eConfig::getInstance()->getKey("/dreamnetcast/minbuffersize", minBufferSize);
	minBufferSize = minBufferSize == 0 ? DFLT_MIN_BUFFER / 1024 : minBufferSize / 1024;

	cmove(ePoint(100, 110));
	cresize(eSize(510, 280));
	setText(_("Settings..."));

	lb_Text1Lbl = new eLabel(this);
	lb_Text1Lbl->move(ePoint(10, 20));
	lb_Text1Lbl->resize(eSize(220, 30));
	lb_Text1Lbl->setText(_("SHOUTcast download limit:"));
	lb_Text1Lbl->loadDeco();
	setWidgetFont(lb_Text1Lbl);

	ef_Limit = new eNumber(this, 1, 1, 300, 3, &limit);
	ef_Limit->move(ePoint(240, 20));
	ef_Limit->resize(eSize(50, 30));
	ef_Limit->loadDeco();
	setWidgetFont(ef_Limit);

	lb_Text2Lbl = new eLabel(this);
	lb_Text2Lbl->move(ePoint(10, 60));
	lb_Text2Lbl->resize(eSize(220, 30));
	lb_Text2Lbl->setText(_("Minimum buffer size (KB):"));
	lb_Text2Lbl->loadDeco();
	setWidgetFont(lb_Text2Lbl);

	ef_MinBufferSize = new eNumber(this, 1, 1, 16, 2, &minBufferSize);
	ef_MinBufferSize->move(ePoint(240, 60));
	ef_MinBufferSize->resize(eSize(50, 30));
	ef_MinBufferSize->loadDeco();
	setWidgetFont(ef_MinBufferSize);

	lb_Text3Lbl = new eLabel(this);
	lb_Text3Lbl->move(ePoint(10, 120));
	lb_Text3Lbl->resize(eSize(clientrect.width() - 20, 30));
	lb_Text3Lbl->setText(_("Press download button to download SHOUTcast playlist."));
	lb_Text3Lbl->loadDeco();
	setWidgetFont(lb_Text3Lbl);

	btnOK = new eButton(this);
	btnOK->setText(_("OK"));
	btnOK->move(ePoint(20, 210));
	btnOK->resize(eSize(110, 30));
	btnOK->setShortcut("green");
    btnOK->setShortcutPixmap("green");
	btnOK->loadDeco();
	setWidgetFont(btnOK);
	CONNECT(btnOK->selected, DreamCastSettingsDialog::save);

	btnDownload = new eButton(this);
	btnDownload->setText(_("Download"));
	btnDownload->move(ePoint(140, 210));
	btnDownload->resize(eSize(110, 30));
	btnDownload->setShortcut("yellow");
    btnDownload->setShortcutPixmap("yellow");
	btnDownload->loadDeco();
	setWidgetFont(btnDownload);
	CONNECT(btnDownload->selected, DreamCastSettingsDialog::download);

	sbStatus = new eStatusBar(this);
	sbStatus->setFlags(eStatusBar::flagOwnerDraw);
	sbStatus->move(ePoint(0, getClientSize().height() - 30));
	sbStatus->resize(eSize(getClientSize().width(), 30));
	sbStatus->loadDeco();

	eDebug("DreamCastSettingsDialog::DreamCastSettingsDialog - end");
}

/*
 * Destructor
 */
DreamCastSettingsDialog::~DreamCastSettingsDialog()
{
	eDebug("DreamCastSettingsDialog::~DreamCastSettingsDialog - start/end");
}

/*
 * Downloads a new playlist
 */
void DreamCastSettingsDialog::download()
{
	eMessageBox mb(
		_("Do you want to replace " CONFIGDIR "/stations.xml?"),
		_("Replace"),
		eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion);

	mb.show();
	int res=mb.exec();
	mb.hide();

	if (res == eMessageBox::btYes)
	{
		int theLimit = ef_Limit->getNumber();

		setStatus(_("Downloading XML..."));

		eString command;

		command.sprintf("wget -O " CONFIGDIR "/stations.xml 'http://www.shoutcast.com/sbin/xmllister.phtml?service=dreamnetcast&no_compress=1&limit=%d'", theLimit);
		int rc = system(command.c_str());

		if (rc == 0)
			setStatus(_("Download complete..."));
		else
			setError(_("Download failed..."));
	}
}

/*
 * Saves the settings
 */
void DreamCastSettingsDialog::save()
{
	eConfig::getInstance()->setKey("/dreamnetcast/minbuffersize", ef_MinBufferSize->getNumber() * 1024);
	reject();
}

/*
 * Sets the status text in the status bar
 */
void DreamCastSettingsDialog::setStatus(const eString &string)
{
	eDebug("DreamCastSettingsDialog::setStatus - start");
	sbStatus->setText(string);
	eDebug("DreamCastSettingsDialog::setStatus - end");
}

/*
 * Error handling routine
 */
void DreamCastSettingsDialog::setError(const char *text)
{
	eDebug("DreamCastSettingsDialog::setError - start");

	eString errmsg = text;
	setStatus(errmsg);

	if (errmsg.length())
	{
		eMessageBox box(errmsg, _("Error!"), eMessageBox::btOK|eMessageBox::iconError);
		box.show();
		box.exec();
		box.hide();
	}

	eDebug("DreamCastSettingsDialog::setError - end");
}

/*
 * DreamCastMinimisedDialog:
 */

/*
 * Constructor
 */
DreamCastMinimisedDialog::DreamCastMinimisedDialog()
{
	eDebug("DreamCastMinimisedDialog::DreamCastMinimisedDialog - start");

	cmove(ePoint(100, 550));
	cresize(eSize(510, 0));
	setText(_("DreamNetCast - Press OK Button To Restore..."));

	btnRestore = new eButton(this);
	CONNECT(btnRestore->selected, eWidget::reject);

	eDebug("DreamCastMinimisedDialog::DreamCastMinimisedDialog - end");
}

/*
 * Destructor
 */
DreamCastMinimisedDialog::~DreamCastMinimisedDialog()
{
	eDebug("DreamCastMinimisedDialog::~DreamCastMinimisedDialog - start/end");
}

/*
 * Plugin entry point, declared to use C calling convention
 */
extern "C" int plugin_exec(PluginParam *par)
{
	DreamCastDialog dlg;
	dlg.show();
	dlg.exec();
	dlg.hide();

	return 0;
}

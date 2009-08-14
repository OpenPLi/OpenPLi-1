/*
 *  DreamNetCast
 *
 *  Author: xphile
 *
 *  This header file contains the definitions needed to implement
 *  the DreamCast plugin
 *
 *   
 *	30/05/2004 - xphile - Initial release
 *	20/07/2004 - xphile - Version 1.0
 */
#ifndef __src_dreamnetcast_h
#define __src_dreamnetcast_h

#define ABOUT_STR			"DreamNetCast v1.0\n\nAuthor: xphile\nHome: www.dreambox.net.au\nSupport: www.sat-industry.net"
#define INPUT_SIZE			8*1024
#define OUTPUT_SIZE			256*1024
#define DFLT_MIN_BUFFER		8*1024

void setWidgetFont(eWidget *widget);

class DreamCastDialog;

class scHTTPStream: public eHTTPDataSource
{
	private:
		eIOBuffer &buffer;
		eString icyName, icyGenre;
		int bytes;
		int metadatainterval, metadataleft, metadatapointer;
		__u8 metadata[16*256+1]; // maximum size
		void processMetaData();

	public:
		scHTTPStream(eHTTPConnection *c, eIOBuffer &buffer);
		~scHTTPStream();
		
		void haveData(void *data, int len);
		Signal0<void> dataAvailable;
		Signal1<void,eString> metaDataUpdated;
		
	friend class StreamDecoder;
};

class StreamDecoder: public Object
{
	private:
		enum
		{
			stateInit, stateError, stateBuffering, stateBufferFull, statePlaying, stateFileEnd
		};
	
		int state;
		int dspfd;
		int maxOutputBufferSize, minOutputBufferSize;
		int error;
		int outputbr;
	
		DreamCastDialog *dialog;
	
		eAudioDecoder *audiodecoder;
		eIOBuffer input, output;
		scHTTPStream *stream;
		eHTTPConnection *http;
		eString http_status;
		eLock poslock;
		eString streamTitle, streamUrl;
		eAudioDecoder::pcmSettings pcmsettings;
		eSocketNotifier *outputsn;
		
		void metaDataUpdated(eString metadata);
		void streamingDone(int err);
		void decodeMoreHTTP();
		void outputReady(int what);
		void checkFlow(int last);
		void dspSync();
		void newAudioStreamIdFound(unsigned int id);
		eHTTPDataSource *createStreamSink(eHTTPConnection *conn);

	public:
		StreamDecoder(const char *filename, DreamCastDialog *theDialog, int minBufferSize);
		~StreamDecoder();
};

/*
 * The dialog class
 */
class DreamCastDialog: public eWindow
{
	private:
		eListBox<eListBoxEntryText> *dummy;
		
		eComboBox	*cbx_Station;
		eLabel		*lb_IcyNameLbl;
		eLabel		*lb_IcyGenreLbl;
		eLabel		*lb_StreamTitleLbl;
		eLabel		*lb_IcyName;
		eLabel		*lb_IcyGenre;
		eLabel		*lb_StreamTitle;
		eButton		*btnPlay;
		eButton		*btnStop;
		eButton		*btnClose;
		eButton		*btnSettings;
		eButton		*btnMinimise;
		eButton		*btnAbout;
		eStatusBar  *sbStatus;
	
		StreamDecoder	*mp3Decoder;
		XMLTreeParser	*directory;
		int				apid;
		
		void playStream();
		void stopStream();
		void updateSettings();
		void about();
		void minimise();
		void initialise();
		void loadStations(XMLTreeNode *root);

	public:
		DreamCastDialog();
		~DreamCastDialog();
		
		void setStatus(const eString &string);
		void setError(const char *text);
		
		void setIcyName(eString &text);
		void setIcyGenre(eString &text);
		void setStreamTitle(eString &text);
};

/*
 * The settings dialog class
 */
class DreamCastSettingsDialog: public eWindow
{
	private:
		eLabel		*lb_Text1Lbl;
		eNumber		*ef_Limit;
		eLabel		*lb_Text2Lbl;
		eNumber		*ef_MinBufferSize;
		eLabel		*lb_Text3Lbl;
		eButton		*btnOK;
		eButton		*btnDownload;
		eStatusBar  *sbStatus;

		int limit;
		int minBufferSize;

		void download();
		void save();
		
	public:
		DreamCastSettingsDialog();
		~DreamCastSettingsDialog();
		
		void setStatus(const eString &string);
		void setError(const char *text);
};

/*
 * The minimised dialog class
 */
class DreamCastMinimisedDialog: public eWindow
{
	private:
		eButton		*btnRestore;

	public:
		DreamCastMinimisedDialog();
		~DreamCastMinimisedDialog();
};

#endif

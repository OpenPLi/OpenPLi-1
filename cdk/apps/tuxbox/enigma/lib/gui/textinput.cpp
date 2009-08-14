#include <lib/base/estring.h>
#include <lib/gui/textinput.h>
#include <lib/gui/actions.h>
#include <lib/gui/guiactions.h>
#include <lib/gui/numberactions.h>
#include <lib/gui/eskin.h>
#include <lib/gui/ewindow.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>
#include <lib/gdi/font.h>

struct texteditActions
{
	eActionMap map;
	eAction capslock, swapnum, insertchar, deletechar, backspace, showHelp;
	texteditActions():
		map("textedit", "enigma global"),
		capslock(map, "capslock", _("enable/disable capslock"), eAction::prioDialog),
		swapnum(map, "swapnum", _("put numbers before/after characters"), eAction::prioDialog),
		insertchar(map, "insertchar", _("insert blank at cursor position"), eAction::prioDialog),
		deletechar(map, "deletechar", _("remove the character at the cursor position"), eAction::prioDialog),
		backspace(map, "backspace", _("remove the character before the cursor position"), eAction::prioDialog),
		showHelp(map, "showHelp", _("shows the textinputfield help"), eAction::prioDialog )
	{
	}
};

eAutoInitP0<texteditActions> i_texteditActions(eAutoInitNumbers::actions, "textedit field actions");

std::map< eString, std::vector<std::pair< eString,eString > > > eTextInputField::keymappings;

eTextInputField::eTextInputField( eWidget *parent, eLabel *descr, eTextInputFieldHelpWidget *hlp, const char *deco )
	:eButton( parent, descr, 1, deco), maxChars(0), lastKey(-1), editMode(false),
	editHelpText(_("press ok to save (help is available)")), nextCharTimer(eApp),
	useableChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ˆ¸"
	"–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔ∞±≤≥¥µ∂∑∏π∫ªºΩæø¿¡¬√ƒ≈∆«»… ÀÃÕŒœ"
	" +-.,:?!\"';_*/()<=>%#@&"), capslock(false), swapNum(false), editLabel(0), helpwidget(hlp)
{
	init_eTextInputField();
}
void eTextInputField::init_eTextInputField()
{
	if ( eConfig::getInstance()->getKey("/ezap/rc/TextInputField/nextCharTimeout", nextCharTimeout ) )
		nextCharTimeout=0;
	CONNECT( nextCharTimer.timeout, eTextInputField::nextChar );
	addActionMap(&i_numberActions->map);
	addActionMap(&i_texteditActions->map);
	flags=0;
	align=eTextPara::dirLeft;
	table=0;
	char *language=0;
	if (!eConfig::getInstance()->getKey("/elitedvb/language", language))
	{
		if (strstr(language,"ru_RU"))
			table=5;
		free(language);
	}
	if ( helpwidget )
		editHelpText=_("press ok to save");
	updateHelpWidget();
}

void eTextInputField::loadKeyMappings()
{
	FILE *in=0;
	char *filename=CONFIGDIR "/enigma/resources/keymappings.xml";
	in=fopen(filename, "rt");
	if (!in)
	{
		char *filename=TUXBOXDATADIR "/enigma/resources/keymappings.xml";
		in=fopen(filename, "rt");
	}
	if (!in)
	{
		eDebug("cannot open keymappings.xml");
		return;
	}

	XMLTreeParser parser("ISO-8859-1");
	char buf[2048];

	int done;
	do
	{
		unsigned int len=fread(buf, 1, sizeof(buf), in);
		done=len<sizeof(buf);
		if (!parser.Parse(buf, len, done))
		{
			eFatal("parse error: %s at line %d",
				parser.ErrorString(parser.GetErrorCode()),
				parser.GetCurrentLineNumber());
			fclose(in);
			return;
		}
	} while (!done);
	fclose(in);

	XMLTreeNode *node=parser.RootNode();
	if (!node)
	{
		eFatal("empty keymappings file?");
		return;
	}
	if (strcmp(node->GetType(), "keymappings"))
	{
		eFatal("not a keymappings file");
		return;
	}
	for (node=node->GetChild(); node; node=node->GetNext())
		if (!strcmp(node->GetType(), "keymapping"))
		{
			const char *code=node->GetAttributeValue("code");
			if (!code)
			{
				eFatal("no code specified");
				return;
			}
			keymappings[code]=std::vector< std::pair<eString,eString> >(12);
			int cnt=0;
			for (XMLTreeNode *xam=node->GetChild(); xam; xam=xam->GetNext())
			{
				if (!strcmp(xam->GetType(), "codes"))
				{
					const char 	*lower(xam->GetAttributeValue("lower")),
											*upper(xam->GetAttributeValue("upper"));
					if (!lower)
					{
						eFatal("no lower specified");
						return;
					}
					if (!upper)
					{
						eFatal("no upper specified");
						return;
					}
					keymappings[code][cnt].first=lower;
					keymappings[code][cnt].second=upper;
				}
				++cnt;
			}
		}
		else
		{
			eFatal("no keymapping specified");
			return;
		}
}

void eTextInputField::updateHelpWidget()
{
	if ( helpwidget )
	{
		eString keysStr;

		for (int i=0; i < 12; ++i )
		{
			char *language=0;
			eConfig::getInstance()->getKey("/elitedvb/language", language);
			if ( language && keymappings.find(language) != keymappings.end() )
			{
				keysStr = capslock ? keymappings[language][i].second :
					keymappings[language][i].first;
			}
			else
			{
				keysStr = capslock ? keymappings["default"][i].second :
					keymappings["default"][i].first;
			}
			if ( language )
				free(language);

			eString tmpStr = convertUTF8DVB(keysStr, table);

			if ( i < 10)
			{
				if ( swapNum )
					tmpStr = (char)(i+0x30) + tmpStr;
				else
					tmpStr += (char)(i+0x30);
			}

			eString filtered;

			for (unsigned int d=0; d < tmpStr.length(); ++d )
				if ( useableChars.find(tmpStr[d]) != eString::npos )
					filtered+=tmpStr[d];

			helpwidget->keys[i]->setText(convertDVBUTF8((unsigned char*)filtered.c_str(),filtered.length(), table));
		}
	}
}

void eTextInputField::updated()
{
	if ( editLabel )
		editLabel->setText(convertDVBUTF8((unsigned char*)isotext.c_str(),isotext.length(), table));
	else
		text=convertDVBUTF8((unsigned char*)isotext.c_str(),isotext.length(), table);
	drawCursor();
}

void eTextInputField::nextChar()
{
	if ( !maxChars || (curPos+1 < (int)maxChars) )
	{
		lastKey=-1;
		++curPos;
		if ( curPos > (int)isotext.length()-1 )
			isotext+=' ';
		updated();
	}
}

void eTextInputField::setUseableChars( const char* uchars )
{
	useableChars=uchars;
	updateHelpWidget();
}

void eTextInputField::setNextCharTimeout( unsigned int newtimeout )
{
	nextCharTimeout=newtimeout;
}

void eTextInputField::drawCursor()
{
//	eDebug("length = %d", isotext.length());
	if ( !cursorRect.isEmpty() )
		eWidget::invalidate(cursorRect);

	cursorRect.setTop((deco_selected?crect_selected.bottom():deco?crect.bottom():clientrect.bottom())-4);

	cursorRect.setHeight( 3 );
	if ( isotext.length() )  // text exist?
	{
		if ( (int)isotext.length() > curPos) // before or on the last char?
		{
			const eRect &bbox = editLabel->getPara()->getGlyphBBox(curPos);
			if ( !bbox.width() )  // Space
			{
				if (curPos)  // char before space?
				{
					const eRect &bbBefore = editLabel->getPara()->getGlyphBBox(curPos-1);
					cursorRect.setLeft( bbBefore.right()+2 );
				}
				if ( (int)isotext.length() > curPos+1) // char after space ?
				{
					const eRect &bbAfter = editLabel->getPara()->getGlyphBBox(curPos+1);
					cursorRect.setRight( bbAfter.left()-2 );
				}
				else  // no char behind Space
					cursorRect.setWidth( 10 );
			}
			else
			{
				cursorRect.setLeft( bbox.left() );
				cursorRect.setWidth( bbox.width() );
			}
		}
		else // we are behind the last character
		{
			const eRect &bbox = editLabel->getPara()->getGlyphBBox(isotext.length()-1);
			cursorRect.setLeft( bbox.right() + ( ( curPos-isotext.length() ) * 10 ) + 2 );
			cursorRect.setWidth( 10 );
		}
	}
	else  //  no one character in text
	{
		cursorRect.setLeft( 0 );
		cursorRect.setWidth( 10 );
	}
	eRect tmp = deco_selected?crect_selected:deco?crect:clientrect;
	if ( cursorRect.right() > scroll.top().second )
	{
		int newpos = scroll.top().first + cursorRect.left();
		scroll.push( std::pair<int,int>( newpos, newpos+tmp.width() ) );
		editLabel->move( ePoint( (-newpos)+tmp.left(), editLabel->getPosition().y() ) );
	}
	else if ( scroll.size() > 1 && cursorRect.left() < scroll.top().first )
	{
		scroll.pop();
		editLabel->move( ePoint( (-scroll.top().first)+tmp.left() , editLabel->getPosition().y() ) );
	}
	cursorRect.moveBy( (deco_selected?crect_selected.left():deco?crect.left():clientrect.left())-scroll.top().first+1, 0 );
	gPainter *painter = getPainter( eRect( ePoint(0,0), size ) );
	painter->setForegroundColor( getForegroundColor() );
	painter->setBackgroundColor( getBackgroundColor() );
	painter->fill( cursorRect );
	if(capslock)
	{
		if ( !capsRect.isEmpty() )
			eWidget::invalidate( capsRect );
		capsRect=cursorRect;
		capsRect.setTop(deco_selected?crect_selected.top():deco?crect.top():clientrect.top());
		capsRect.setHeight( 3 );
		painter->fill( capsRect );
	}
	if (deco_selected && have_focus)
		deco_selected.drawDecoration(painter, ePoint(width(), height()));
	else if (deco)
		deco.drawDecoration(painter, ePoint(width(), height()));

	delete painter;
}

void eTextInputField::setState(int enable, int cancel)
{
	nextCharTimer.stop();
	if (!enable)
	{
		editLabel->hide();
		if (!cancel)
			setText(editLabel->getText());
		else
			setText(oldText);

		delete editLabel;
		editLabel=0;

		setHelpText(oldHelpText);

		while ( text.length() && text[text.length()-1] == ' ' )
			text.erase( text.length()-1 );

		editMode=false;

		if (!cancel)
			/* emit */ selected();

		eRCInput::getInstance()->setKeyboardMode(eRCInput::kmNone);
		eWindow::globalCancel(eWindow::ON);
	}
	else
	{
		oldText=text;
		editMode=true;
		/* emit */ selected();
		capslock=0;
		while(scroll.size())
			scroll.pop();
		eRect tmp = deco_selected?crect_selected:deco?crect:clientrect;
		editLabel=new eLabel(this,0,0);
		editLabel->hide();
		editLabel->move(tmp.topLeft());
		scroll.push( std::pair<int,int>(0,tmp.width()) );
		eSize tmpSize=tmp.size();
		tmpSize.setWidth( tmp.width()*5 );
		editLabel->resize(tmpSize);
		editLabel->setText(text.removeChars('\x86').
			removeChars('\x87').removeChars('\xC2').
			removeChars('\x05'));
		oldHelpText=helptext;
		setText("");
		editLabel->show();
		setHelpText(editHelpText);
		curPos=0;
		drawCursor();
		eRCInput::getInstance()->setKeyboardMode(eRCInput::kmAscii);
		eWindow::globalCancel(eWindow::OFF);
	}
}

int eTextInputField::eventHandler( const eWidgetEvent &event )
{
	if (editLabel)
		isotext=convertUTF8DVB(editLabel->getText(), table);
	else
		isotext=convertUTF8DVB(text, table);
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
		{
			if ( curPos < 0 )
				curPos=0;
//			eDebug("curPos=%d, isotext.length=%d",curPos, isotext.length());
			int key = -1;
			if ( event.action == &i_texteditActions->capslock && editMode)
			{
				capslock^=1;
				if ( capslock )
					drawCursor();
				else if ( !capsRect.isEmpty() )
					eWidget::invalidate( capsRect );
				updateHelpWidget();
			}
			else if ( event.action == &i_texteditActions->swapnum && editMode)
			{
				swapNum^=1;
				updateHelpWidget();
			}
			else if (event.action == &i_texteditActions->insertchar && editMode)
			{
				if ( isotext.length() && (!maxChars || (isotext.length() < maxChars)) )
				{
					lastKey=-1;
					isotext.insert( curPos, " ");
					updated();
				}
			}
			else if (event.action == &i_texteditActions->deletechar && editMode)
			{
//				eDebug("istext.length is %d, curPos is %d", isotext.length(), curPos);
				if ( isotext.length() )
				{
					lastKey=-1;
					isotext.erase( curPos, 1 );
//					eDebug("curPos=%d, length=%d", curPos, text.length() );
					if ( (int)isotext.length() == curPos )
					{
//						eDebug("curPos--");
						--curPos;
					}
					updated();
				}
			}
			else if (event.action == &i_texteditActions->backspace && editMode)
			{
//				eDebug("istext.length is %d, curPos is %d", isotext.length(), curPos);
				if (isotext.length())
				{
					lastKey=-1;
					if (curPos)
						curPos--;
					isotext.erase( curPos, 1 );
					updated();
				}
			}
			else if ( event.action == &i_texteditActions->showHelp && !helpwidget && editMode )
			{
				eTextInputFieldHelpWindow wnd;
				helpwidget = wnd.helpwidget;
				updateHelpWidget();
				eWindow::globalCancel(eWindow::ON);
				wnd.show();
				wnd.exec();
				wnd.hide();
				helpwidget=0;
				drawCursor();
				eWindow::globalCancel(eWindow::OFF);
			}
			else if ( (event.action == &i_cursorActions->up ||
				event.action == &i_cursorActions->down) && editMode )
			{
				lastKey=-1;
				nextCharTimer.stop();
				const char *pc1=useableChars.c_str();
				const char *pc2=strchr( pc1, isotext[curPos] );

				if( !pc2 || !pc2[0] )
					pc2=pc1;

				if(event.action == &i_cursorActions->up)
				{
					pc2++;
					if(!*pc2)
						pc2=pc1;
				}
				else
				{
					if(pc2==pc1)
						while(*pc2)
							pc2++;
					pc2--;
				}
				if ( isotext.length() )
					isotext[curPos] = *pc2;
				else
					isotext+=*pc2;
				updated();
			}
			else if (event.action == &i_cursorActions->left && editMode )
			{
				nextCharTimer.stop();
				if ( curPos > 0 )
				{
					--curPos;
					lastKey=-1;
					updated();
				}
			}
			else if (event.action == &i_cursorActions->right && editMode)
			{
				nextCharTimer.stop();
				nextChar();
			}
			else if (event.action == &i_cursorActions->ok)
				setState(!editMode, 0);
			else if ( editMode && event.action == &i_cursorActions->cancel )
			{
				setState(0, 1);
				if ( flags & flagCloseParent && parent )
					parent->reject();
				break;
			}
			else if (event.action == &i_numberActions->key0 && editMode )
				key=0;
			else if (event.action == &i_numberActions->key1 && editMode )
				key=1;
			else if (event.action == &i_numberActions->key2 && editMode )
				key=2;
			else if (event.action == &i_numberActions->key3 && editMode )
				key=3;
			else if (event.action == &i_numberActions->key4 && editMode )
				key=4;
			else if (event.action == &i_numberActions->key5 && editMode )
				key=5;
			else if (event.action == &i_numberActions->key6 && editMode )
				key=6;
			else if (event.action == &i_numberActions->key7 && editMode )
				key=7;
			else if (event.action == &i_numberActions->key8 && editMode )
				key=8;
			else if (event.action == &i_numberActions->key9 && editMode )
				key=9;
			else if (event.action == &i_numberActions->keyExt1 && editMode )
				key=10;
			else if (event.action == &i_numberActions->keyExt2 && editMode )
				key=11;
			else
				return eButton::eventHandler( event );
			if ( (lastKey != -1 && key != -1) &&
				(key != lastKey || flags & flagGoAlwaysNext) )
			{
				if ( nextCharTimer.isActive() )
					nextCharTimer.stop();
				nextChar();
			}
			if ( editMode && key != -1 )
			{
				eString keysStr;
				char *language=0;
				eConfig::getInstance()->getKey("/elitedvb/language", language);
				if ( language && keymappings.find(language) != keymappings.end() )
				{
					keysStr = capslock ? keymappings[language][key].second :
						keymappings[language][key].first;
				}
				else
				{
					keysStr = capslock ? keymappings["default"][key].second :
						keymappings["default"][key].first;
				}
				if ( language )
					free(language);

				eString tmpStr=convertUTF8DVB(keysStr, table);

				if ( key < 10)
				{
					if ( swapNum )
						tmpStr = (char)(key+0x30) + tmpStr;
					else
						tmpStr += (char)(key+0x30);
				}

				const char *keys = tmpStr.c_str();

				char newChar = 0;
				if ( key == lastKey )
				{
					char *oldkey = strchr( keys, isotext[curPos] );
					newChar = oldkey?keys[oldkey-keys+1]:0;
				}

				if (!newChar)
					newChar = keys[0];

//				eDebug("newChar = %d", newChar );
				char testChar = newChar;
				do
				{
					if ( strchr( useableChars.c_str(), newChar ) ) // char useable?
					{
						if ( curPos == (int)isotext.length() )
							isotext += newChar;
						else
							isotext[curPos] = newChar;
						updated();
						if(nextCharTimeout)
							nextCharTimer.start(nextCharTimeout,true);
						break;
					}
					else
					{
						nextCharTimer.stop();
						char *oldkey = strchr( keys, newChar );
						newChar=oldkey?keys[oldkey-keys+1]:0;
						if (!newChar)
							newChar=keys[0];
					}
				}
				while( newChar != testChar );  // all chars tested.. and no char is useable..
				lastKey=key;
			}
		}
		break;

		default:
			return eButton::eventHandler( event );
		break;
	}
	return 1;
}

void eTextInputField::lostFocus()
{
	if ( editMode )
	{
		nextCharTimer.stop();
		isotext=convertUTF8DVB(editLabel->getText(), table);
		setText(isotext);
		setHelpText(oldHelpText);
		while ( text.length() && text[text.length()-1] == ' ' )
			text.erase( text.length()-1 );
		editMode=false;
		/* emit */ selected();
		delete editLabel;
		editLabel=0;
	}
	eWindow::globalCancel(eWindow::ON);
	eButton::lostFocus();
}

void eTextInputField::redrawWidget( gPainter *target, const eRect &area )
{
	eButton::redrawWidget( target, area );
}

void eTextInputFieldHelpWidget::redrawWidget(gPainter *target, const eRect & area )
{
	int fieldwidth = (size.width()-4) / 3;
	target->setForegroundColor( eSkin::getActive()->queryColor("textinput.helpwindow.border") );
	target->fill(eRect(0,0,size.width(),2));
	target->fill(eRect(0,0,2,size.height()));
	target->fill(eRect(size.width()-2,0,2,size.height()));
	target->fill(eRect(0,size.height()-2,size.width(),2));
	target->fill(eRect(0,44,size.width(),2));
	target->fill(eRect(0,89,size.width(),2));
	target->fill(eRect(0,133,size.width(),2));
	target->fill(eRect(0,size.height()-77,size.width(),2));
	target->fill(eRect(0,size.height()-40,size.width(),2));
	target->fill(eRect(1+fieldwidth,0,2,size.height()-75));
	target->fill(eRect(1+fieldwidth*2,0,2,size.height()-75));
	target->fill(eRect(1+size.width()/2,size.height()-75,2,size.height()));
}

eTextInputFieldHelpWidget::eTextInputFieldHelpWidget(eWidget *parent)
	:eWidget(parent, 0)
{
	keys[0] = new eLabel(this); keys[0]->setName("zero");
	keys[1] = new eLabel(this); keys[1]->setName("one");
	keys[2] = new eLabel(this); keys[2]->setName("two");
	keys[3] = new eLabel(this); keys[3]->setName("three");
	keys[4] = new eLabel(this); keys[4]->setName("four");
	keys[5] = new eLabel(this); keys[5]->setName("five");
	keys[6] = new eLabel(this); keys[6]->setName("six");
	keys[7] = new eLabel(this); keys[7]->setName("seven");
	keys[8] = new eLabel(this); keys[8]->setName("eight");
	keys[9] = new eLabel(this); keys[9]->setName("nine");
	keys[10] = new eLabel(this); keys[10]->setName("special_one");
	keys[11] = new eLabel(this); keys[11]->setName("special_two");
	if (eSkin::getActive()->build(this, "eTextInputFieldHelpWidget"))
		eWarning("TextInputFieldHelpWidget build failed!");
}

int eTextInputField::keyDown(int rc)
{
	if (editMode & (rc >= KEY_ASCII))
	{
		int ascii = rc - KEY_ASCII;
		eDebug("ascii is %d (%c)", ascii, ascii);
		if ( strchr( useableChars.c_str(), ascii ) ) // char useable?
		{
			if ( curPos == (int)isotext.length() )
				isotext += ascii;
			else
				isotext[curPos] = ascii;
			updated();
			nextChar();
			return 1;
		}
	}
	return 0;
}

static eWidget *create_eTextInputField(eWidget *parent)
{
	return new eTextInputField(parent);
}

class eTextInputFieldSkinInit
{
public:
	eTextInputFieldSkinInit()
	{
		eSkin::addWidgetCreator("eTextInputField", create_eTextInputField);
		eTextInputField::loadKeyMappings();
	}
	~eTextInputFieldSkinInit()
	{
		eSkin::removeWidgetCreator("eTextInputField", create_eTextInputField);
	}
};

eAutoInitP0<eTextInputFieldSkinInit> init_eTextInputFieldSkinInit(25, "eTextInputField");


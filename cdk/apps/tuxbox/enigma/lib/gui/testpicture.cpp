#include <lib/gui/testpicture.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gdi/font.h>
#include <lib/gui/guiactions.h>
#include <lib/gui/numberactions.h>

void eTestPicture::hideDesc()
{
	if (testmode != testFUBK)
		description->hide();
}

int eTestPicture::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if (event.action == &i_cursorActions->ok)
			close(-1);
		else if (event.action == &i_cursorActions->cancel)
			close(-1);
		else if (event.action == &i_numberActions->key0)
			close(0);
		else if (event.action == &i_numberActions->key1)
			close(1);
		else if (event.action == &i_numberActions->key2)
			close(2);
		else if (event.action == &i_numberActions->key3)
			close(3);
		else if (event.action == &i_numberActions->key4)
			close(4);
		else if (event.action == &i_numberActions->key5)
			close(5);
		else if (event.action == &i_numberActions->key6)
			close(6);
		else if (event.action == &i_numberActions->key7)
			close(7);
		else if (event.action == &i_numberActions->key8)
			close(8);
		else if (event.action == &i_numberActions->key9)
			close(9);
		else
			break;
		return 0;
	case eWidgetEvent::willShow:
		desctimer.start(1000);
		break;
	default:
		break;
	}
	return eWidget::eventHandler(event);
}

void eTestPicture::redrawWidget(gPainter *target, const eRect &area)
{
	switch (testmode)
	{
	case testColorbar:
	{
		for (int i=0; i<8; ++i)
		{
			gColor c=basic[i];
			
			target->setForegroundColor(c);
			target->fill(eRect(720/8*i, 0, 720/8, 576));
		}
		break;
	}
	case testGray:
	{
		for (int i=0; i<8; ++i)
		{
			gColor c=gray+i;
			
			target->setForegroundColor(c);
			target->fill(eRect(720/8*i, 0, 720/8, 576));
		}
		break;
	}
	case testFUBK:
	{
				// background
		target->setBackgroundColor(gray+2);
		target->clear();
		
				// white lines
		target->setForegroundColor(white);
		for (int x=6; x<720; x+=44)
			target->fill(eRect(x, 0, 3, 576));
		for (int y=34; y<576; y+=44)
			target->fill(eRect(0, y, 720, 3));
		
		for (int i=0; i<8; ++i)
		{
			target->setForegroundColor(basic[i]);
			target->fill(eRect(140+i*55, 80, 55, 80));
			target->setForegroundColor(gray+i);
			target->fill(eRect(140+i*55, 160, 55, 80));
		}
#if 1
		int freq;
		int phase=0;
		for (int x=0; x<440; x+=freq)
		{
			freq=(440-x)/44 + 1;
			target->setForegroundColor(phase ? white : black);
			target->fill(eRect(140+x, 320, freq, 160));
			phase = !phase;
		}
#endif
#if 0
		double phase=0;
		for (int x=0; x<440; ++x)
		{
			// freq = 0.5 .. 5Mhz -> 10 Pixel ~= 1Mhz
			double inc = (x/440.0)*(x/440.0) * 3.141 * 2 / 5.0 + 0.5;
			phase += inc;
			int s = int((sin(phase) + 1) * 4);
			if (s < 0)
				s = 0;
			if (s > 7)
				s = 7;
			target->setForegroundColor(gray + s);
			target->fill(eRect(140+x, 320, 1, 160));
		}
#endif

		break;
	}
	case testRed:
		target->setBackgroundColor(red);
		target->clear();
		break;
	case testGreen:
		target->setBackgroundColor(green);
		target->clear();
		break;
	case testBlue:
		target->setBackgroundColor(blue);
		target->clear();
		break;
	case testWhite:
		target->setBackgroundColor(white);
		target->clear();
		break;
	case testBlack:
		target->setBackgroundColor(black);
		target->clear();
		break;
	}
}

eTestPicture::eTestPicture(int testmode): eWidget(0, 1), testmode(testmode), desctimer(eApp)
{
	init_eTestPicture();
}
void eTestPicture::init_eTestPicture()
{
	red = eSkin::getActive()->queryColor("std_red");
	green = eSkin::getActive()->queryColor("std_green");
	blue = eSkin::getActive()->queryColor("std_blue");
	black = eSkin::getActive()->queryColor("std_black");
	white = eSkin::getActive()->queryColor("std_white");
	gray = eSkin::getActive()->queryColor("std_gray");
	setBackgroundColor(-1);
	
	CONNECT(desctimer.timeout, eTestPicture::hideDesc);
	
	basic[0] = white;
	basic[1] = eSkin::getActive()->queryColor("std_dyellow");
	basic[2] = eSkin::getActive()->queryColor("std_dcyan");
	basic[3] = eSkin::getActive()->queryColor("std_dgreen");
	basic[4] = eSkin::getActive()->queryColor("std_dmagenta");
	basic[5] = eSkin::getActive()->queryColor("std_dred");
	basic[6] = eSkin::getActive()->queryColor("std_dblue");
	basic[7] = black;
	
	addActionMap(&i_numberActions->map);
	addActionMap(&i_cursorActions->map);

	description = new eLabel(this, eLabel::flagVCenter);
	switch (testmode)
	{
	case testColorbar:
	case testRed:
	case testBlue:
	case testGreen:
	case testWhite:
	case testBlack:
	case testGray:
		description->move(ePoint(100, 100));
		description->resize(eSize(200, 30));
		description->setBackgroundColor(black);
		description->setForegroundColor(white);
		break;
	case testFUBK:
		description->move(ePoint(140, 240));
		description->resize(eSize(440, 80));
		description->setBackgroundColor(black);
		description->setForegroundColor(white);
		description->setText("ENIGMA");
		description->setAlign(eTextPara::dirCenter);
		break;
	default:
		break;
	}
	switch (testmode)
	{
	case testColorbar: description->setText("COLORBAR"); break;
	case testRed:      description->setText("RED"); break;
	case testBlue:     description->setText("BLUE"); break;
	case testGreen:    description->setText("GREEN"); break;
	case testWhite:    description->setText("WHITE (100%)"); break;
	case testGray:     description->setText("0% TO 100%"); break;
	case testBlack:    description->setText("BLACK (0%)"); break;
	}
	description->show();

	/* help text for calibration screen */
	setHelpText(_("Calibration Pictures\n\n->[MENU] ->(6)Settings; ->Video Calibration\n. . . . . . . . . .\n\n" \
								"Test pattern with which you can adjust your TV set.\n\nNot for commercial use!\n. . . . . . . . . .\n" \
								"Usage:\n\n[OK]\tConfirm warning\n\n[DIGITS]\tSelect test pattern\n\n[1]\tRed\n\n[2]\tGreen\n\n" \
								"[3]\tBlue\n\n[4]\tColorbars\n\n[5]\tWhite\n\n[6]\tTest image\n\n\tGrey pallet\n\n[8]\tBlack\n\n[EXIT]\tClose window"));
}

int eTestPicture::display(int mode)
{
	int res;
	eTestPicture test(mode);
	test.move(ePoint(0, 0));
	test.resize(eSize(720, 576));
	test.show();
	res = test.exec();
	test.hide();
	return res;
}

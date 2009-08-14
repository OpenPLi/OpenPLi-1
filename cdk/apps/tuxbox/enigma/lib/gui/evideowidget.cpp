#if HAVE_DVB_API_VERSION < 3
#include <dbox/avia_gt_pig.h>
#define PIG "/dev/dbox/pig0"
#else
#include <linux/input.h>
#include <linux/videodev.h>
#define PIG "/dev/v4l/video0"
#endif
#include <fcntl.h>
#include <sys/ioctl.h>

#include <lib/gui/evideowidget.h>
#include <lib/gui/eskin.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/driver/eavswitch.h>

eVideoWidget::eVideoWidget(eWidget *parent)
: eWidget(parent)
{
	pigDevice = -1;
	borderWidth = 0;
}

eVideoWidget::~eVideoWidget()
{
}

void eVideoWidget::calculatePigSizes(eSize limits)
{
	pigSize = limits;
	FILE *bitstream = fopen("/proc/bus/bitstream", "rt");
	if (bitstream)
	{
		int frate = 0;
		int aspect = 0;
		char buffer[100];
		while (fgets(buffer, 100, bitstream))
		{
			if (!strncmp(buffer, "A_RATIO: ", 9))
			{
				aspect = atoi(buffer + 9);
			}
			if (!strncmp(buffer, "F_RATE: ", 8))
			{
				frate = atoi(buffer + 8);
			}
		}

		int ratio;
		bool ntsc = false;
		switch (frate)
		{
		default:
		case 1:
		case 2:
		case 3:
		case 6:
			/* PAL */
			ratio = 125;
			break;
		case 4:
		case 5:
		case 7:
		case 8:
			/* NTSC */
			/* we use the same ratio for NTSC, but we adjust the 'visible' pig height */
			ratio = 125;
			ntsc = true;
			break;
		}
		if (aspect >= 3)
		{
			/* anamorph */
			ratio *= 12;
			ratio /= 10;
		}
		pigSize.setHeight(limits.width() * 100 / ratio);
		visiblePigSize = pigSize;
		if (ntsc)
		{
			/*
			 * On NTSC, the lower fifth of the PiG shows garbage,
			 * so we reduce the visible PiG height, to hide it.
			 */
			ratio *= 12;
			ratio /= 10;
			visiblePigSize.setHeight(limits.width() * 100 / ratio);
		}
		fclose(bitstream);
	}

	eDebug("pigSize %d, %d", pigSize.width(), pigSize.height());
	eDebug("visiblePigSize %d, %d", visiblePigSize.width(), visiblePigSize.height());
}

int eVideoWidget::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::willShow:
		{
			if (pigDevice >= 0)
			{
				close(pigDevice);
			}
			pigDevice = open(PIG, O_RDWR);

			calculatePigSizes(eSize(size.width() - 2 * borderWidth, size.height() - 2 * borderWidth));
			ePoint pigposition(position.x() + borderWidth, position.y() + borderWidth);

#if HAVE_DVB_API_VERSION < 3
			avia_pig_hide(pigDevice);
			avia_pig_set_pos(pigDevice, pigposition.x(), pigposition.y());
			avia_pig_set_size(pigDevice, pigSize.width(), pigSize.height());
			avia_pig_set_stack(pigDevice, 2);
			avia_pig_show(pigDevice);
#else
			struct v4l2_format format;
			int sm = 0;
			ioctl(pigDevice, VIDIOC_OVERLAY, &sm);
			sm = 1;
			myclip.c.left = 0;
			myclip.c.top = 0;
			myclip.c.width = PigW;
			myclip.c.height = PigH;

			myclip.next = NULL;
			ioctl(pigDevice, VIDIOC_G_FMT, &format);
			format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
			format.fmt.win.w.left   = pigposition.x();
			format.fmt.win.w.top    = pigposition.y();
			format.fmt.win.w.width  = pigSize.width();
			format.fmt.win.w.height = pigSize.height();
			ioctl(pigDevice, VIDIOC_S_FMT, &format);
			ioctl(pigDevice, VIDIOC_OVERLAY, &sm);
	#endif
		}
		break;
	case eWidgetEvent::willHide:
		if (pigDevice >= 0)
		{
#if HAVE_DVB_API_VERSION < 3
			avia_pig_hide(pigDevice);
#else
			int screenmode = 0;
			ioctl(pigDevice, VIDIOC_OVERLAY, &screenmode);
#endif
			close(pigDevice);
			pigDevice = -1;
		}
		break;
	case eWidgetEvent::changedPosition:
		if (pigDevice >= 0)
		{
			ePoint pigposition(position.x() + borderWidth, position.y() + borderWidth);
#if HAVE_DVB_API_VERSION < 3
			avia_pig_set_pos(pigDevice, pigposition.x(), pigposition.y());
#else
			struct v4l2_format format;
			ioctl(pigDevice, VIDIOC_G_FMT, &format);
			format.fmt.win.w.left   = pigposition.x();
			format.fmt.win.w.top    = pigposition.y();
			ioctl(pigDevice, VIDIOC_S_FMT, &format);
#endif
		}
		break;
	case eWidgetEvent::changedSize:
		if (pigDevice >= 0)
		{
			calculatePigSizes(eSize(size.width() - 2 * borderWidth, size.height() - 2 * borderWidth));
#if HAVE_DVB_API_VERSION < 3
			avia_pig_set_size(pigDevice, pigSize.width(), pigSize.height());
#else
			struct v4l2_format format;
			ioctl(pigDevice, VIDIOC_G_FMT, &format);
			format.fmt.win.w.width  = pigSize.width();
			format.fmt.win.w.height = pigSize.height();
			ioctl(pigDevice, VIDIOC_S_FMT, &format);
#endif
		}
		break;
	default:
		return eWidget::eventHandler(event);
	}
}

void eVideoWidget::redrawWidget(gPainter *target, const eRect &area)
{
	if (borderWidth && getForegroundColor())
	{
		/* draw borderWidth */
		target->setForegroundColor(getForegroundColor());
		target->fill(eRect(0, 0, size.width(), borderWidth));
		target->fill(eRect(0, borderWidth, borderWidth, size.height() - borderWidth));
		target->fill(eRect(borderWidth, size.height() - borderWidth, size.width() - borderWidth, borderWidth));
		target->fill(eRect(size.width() - borderWidth, borderWidth, borderWidth, size.height() - borderWidth));
	}

	if (getBackgroundColor())
	{
		target->setForegroundColor(gColor(0));
		target->fill(eRect(borderWidth, borderWidth, visiblePigSize.width(), visiblePigSize.height()));
	}
}

int eVideoWidget::setProperty(const eString &prop, const eString &value)
{
	if (prop == "border")
		borderWidth = atoi(value.c_str());
	else
		return eWidget::setProperty(prop, value);
	return 0;
}

static eWidget *create_eVideoWidget(eWidget *parent)
{
	return new eVideoWidget(parent);
}

class eVideoWidgetSkinInit
{
public:
	eVideoWidgetSkinInit()
	{
		eSkin::addWidgetCreator("eVideoWidget", create_eVideoWidget);
	}
	~eVideoWidgetSkinInit()
	{
		eSkin::removeWidgetCreator("eVideoWidget", create_eVideoWidget);
	}
};

eAutoInitP0<eVideoWidgetSkinInit> init_eVideoWidgetSkinInit(eAutoInitNumbers::guiobject, "eVideoWidget");

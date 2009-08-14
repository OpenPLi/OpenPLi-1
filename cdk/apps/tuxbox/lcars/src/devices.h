#ifndef DEVICES_H
#define DEVICES_H

#include <config.h>

#include <ost/dmx.h>
#include <ost/video.h>
#include <ost/frontend.h>
#include <ost/audio.h>
#include <ost/sec.h>
#include <ost/ca.h>
#include <linux/videodev.h>
#include <dbox/avia_gt_pig.h>
#include <dbox/avs_core.h>
#include <dbox/fp.h>
#include <dbox/info.h>

#define VIDEO_DEV "/dev/dvb/card0/video0"
#define AUDIO_DEV "/dev/dvb/card0/audio0"
#define DEMUX_DEV "/dev/dvb/card0/demux0"
#define FRONTEND_DEV "/dev/dvb/card0/frontend0"
#define CAM_DEV "/dev/dvb/card0/ca0"
#define PIG_DEV "/dev/v4l/video"
#define AVS_DEV "/dev/dbox/avs0"
#define fe_code_rate_t CodeRate
#define audio_status audioStatus
#define video_status videoStatus
#define dmx_pes_filter_params dmxPesFilterParams
#define dmx_sct_filter_params dmxSctFilterParams
#define dmx_filter dmxFilter
#define video_stream_source_t videoStreamSource_t
#define audio_stream_source_t audioStreamSource_t

#endif


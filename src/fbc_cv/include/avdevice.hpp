// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVDEVICE_HPP_
#define FBC_CV_AVDEVICE_HPP_

// reference: ffmpeg 4.2
//            libavdevice/avdevice.h

#ifdef _MSC_VER

#include "core/fbcdef.hpp"
#include "ffmpeg_common.hpp"

namespace fbc {

enum AVAppToDevMessageType {
	/**
	* Dummy message.
	*/
	AV_APP_TO_DEV_NONE = MKBETAG('N', 'O', 'N', 'E'),

	/**
	* Window size change message.
	*
	* Message is sent to the device every time the application changes the size
	* of the window device renders to.
	* Message should also be sent right after window is created.
	*
	* data: AVDeviceRect: new window size.
	*/
	AV_APP_TO_DEV_WINDOW_SIZE = MKBETAG('G', 'E', 'O', 'M'),

	/**
	* Repaint request message.
	*
	* Message is sent to the device when window has to be repainted.
	*
	* data: AVDeviceRect: area required to be repainted.
	*       NULL: whole area is required to be repainted.
	*/
	AV_APP_TO_DEV_WINDOW_REPAINT = MKBETAG('R', 'E', 'P', 'A'),

	/**
	* Request pause/play.
	*
	* Application requests pause/unpause playback.
	* Mostly usable with devices that have internal buffer.
	* By default devices are not paused.
	*
	* data: NULL
	*/
	AV_APP_TO_DEV_PAUSE = MKBETAG('P', 'A', 'U', ' '),
	AV_APP_TO_DEV_PLAY = MKBETAG('P', 'L', 'A', 'Y'),
	AV_APP_TO_DEV_TOGGLE_PAUSE = MKBETAG('P', 'A', 'U', 'T'),

	/**
	* Volume control message.
	*
	* Set volume level. It may be device-dependent if volume
	* is changed per stream or system wide. Per stream volume
	* change is expected when possible.
	*
	* data: double: new volume with range of 0.0 - 1.0.
	*/
	AV_APP_TO_DEV_SET_VOLUME = MKBETAG('S', 'V', 'O', 'L'),

	/**
	* Mute control messages.
	*
	* Change mute state. It may be device-dependent if mute status
	* is changed per stream or system wide. Per stream mute status
	* change is expected when possible.
	*
	* data: NULL.
	*/
	AV_APP_TO_DEV_MUTE = MKBETAG(' ', 'M', 'U', 'T'),
	AV_APP_TO_DEV_UNMUTE = MKBETAG('U', 'M', 'U', 'T'),
	AV_APP_TO_DEV_TOGGLE_MUTE = MKBETAG('T', 'M', 'U', 'T'),

	/**
	* Get volume/mute messages.
	*
	* Force the device to send AV_DEV_TO_APP_VOLUME_LEVEL_CHANGED or
	* AV_DEV_TO_APP_MUTE_STATE_CHANGED command respectively.
	*
	* data: NULL.
	*/
	AV_APP_TO_DEV_GET_VOLUME = MKBETAG('G', 'V', 'O', 'L'),
	AV_APP_TO_DEV_GET_MUTE = MKBETAG('G', 'M', 'U', 'T'),
};

typedef struct AVDeviceRect {
	int x;      /**< x coordinate of top left corner */
	int y;      /**< y coordinate of top left corner */
	int width;  /**< width */
	int height; /**< height */
} AVDeviceRect;

enum AVDevToAppMessageType {
	/**
	* Dummy message.
	*/
	AV_DEV_TO_APP_NONE = MKBETAG('N', 'O', 'N', 'E'),

	/**
	* Create window buffer message.
	*
	* Device requests to create a window buffer. Exact meaning is device-
	* and application-dependent. Message is sent before rendering first
	* frame and all one-shot initializations should be done here.
	* Application is allowed to ignore preferred window buffer size.
	*
	* @note: Application is obligated to inform about window buffer size
	*        with AV_APP_TO_DEV_WINDOW_SIZE message.
	*
	* data: AVDeviceRect: preferred size of the window buffer.
	*       NULL: no preferred size of the window buffer.
	*/
	AV_DEV_TO_APP_CREATE_WINDOW_BUFFER = MKBETAG('B', 'C', 'R', 'E'),

	/**
	* Prepare window buffer message.
	*
	* Device requests to prepare a window buffer for rendering.
	* Exact meaning is device- and application-dependent.
	* Message is sent before rendering of each frame.
	*
	* data: NULL.
	*/
	AV_DEV_TO_APP_PREPARE_WINDOW_BUFFER = MKBETAG('B', 'P', 'R', 'E'),

	/**
	* Display window buffer message.
	*
	* Device requests to display a window buffer.
	* Message is sent when new frame is ready to be displayed.
	* Usually buffers need to be swapped in handler of this message.
	*
	* data: NULL.
	*/
	AV_DEV_TO_APP_DISPLAY_WINDOW_BUFFER = MKBETAG('B', 'D', 'I', 'S'),

	/**
	* Destroy window buffer message.
	*
	* Device requests to destroy a window buffer.
	* Message is sent when device is about to be destroyed and window
	* buffer is not required anymore.
	*
	* data: NULL.
	*/
	AV_DEV_TO_APP_DESTROY_WINDOW_BUFFER = MKBETAG('B', 'D', 'E', 'S'),

	/**
	* Buffer fullness status messages.
	*
	* Device signals buffer overflow/underflow.
	*
	* data: NULL.
	*/
	AV_DEV_TO_APP_BUFFER_OVERFLOW = MKBETAG('B', 'O', 'F', 'L'),
	AV_DEV_TO_APP_BUFFER_UNDERFLOW = MKBETAG('B', 'U', 'F', 'L'),

	/**
	* Buffer readable/writable.
	*
	* Device informs that buffer is readable/writable.
	* When possible, device informs how many bytes can be read/write.
	*
	* @warning Device may not inform when number of bytes than can be read/write changes.
	*
	* data: int64_t: amount of bytes available to read/write.
	*       NULL: amount of bytes available to read/write is not known.
	*/
	AV_DEV_TO_APP_BUFFER_READABLE = MKBETAG('B', 'R', 'D', ' '),
	AV_DEV_TO_APP_BUFFER_WRITABLE = MKBETAG('B', 'W', 'R', ' '),

	/**
	* Mute state change message.
	*
	* Device informs that mute state has changed.
	*
	* data: int: 0 for not muted state, non-zero for muted state.
	*/
	AV_DEV_TO_APP_MUTE_STATE_CHANGED = MKBETAG('C', 'M', 'U', 'T'),

	/**
	* Volume level change message.
	*
	* Device informs that volume level has changed.
	*
	* data: double: new volume with range of 0.0 - 1.0.
	*/
	AV_DEV_TO_APP_VOLUME_LEVEL_CHANGED = MKBETAG('C', 'V', 'O', 'L'),
};

FBC_EXPORTS void avdevice_register_all(void);
int avdevice_dev_to_app_control_message(struct AVFormatContext *s, enum AVDevToAppMessageType type, void *data, size_t data_size);

} // namespace fbc


#endif // _MSC_VER
#endif // FBC_CV_AVDEVICE_HPP_

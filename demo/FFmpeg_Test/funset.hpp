#ifndef FBC_FFMPEG_TEST_FUNSET_HPP_
#define FBC_FFMPEG_TEST_FUNSET_HPP_

#include <map>
#include <string>
#include <vector>

typedef enum video_codec_type_t {
	VIDEO_CODEC_TYPE_H264,
	VIDEO_CODEC_TYPE_H265,
	VIDEO_CODEC_TYPE_MJPEG,
	VIDEO_CODEC_TYPE_RAWVIDEO
} video_codec_type_t;

/////////////////////////// FFmpeg /////////////////////////////
int test_ffmpeg_rtsp_client_decode_show(); // rtsp decode show
int test_ffmpeg_stream_show_two_thread(); // only support rawvideo encode, two thread
int test_ffmpeg_stream_show(); // only support rawvideo encode
int test_ffmpeg_decode_show_old(); // deprecated interface
int test_ffmpeg_decode_show_new(); // new interface
int test_ffmpeg_decode_show();
int test_ffmpeg_usb_stream();
int test_ffmpeg_rtsp_client();
int test_ffmpeg_decode_dshow();
int test_ffmpeg_dshow_mjpeg_encode_libyuv_decode();

// libavfilter

// libavdevice
int test_ffmpeg_libavdevice_device_list();

// libavformat

// libavcodec

// libswresample
int test_ffmpeg_libswresample_resample(); // audio resample

// libswscale
int test_ffmpeg_libswscale_scale(); // image scale
int test_ffmpeg_libswscale_colorspace(); // color space convert
int test_ffmpeg_libswscale_bgr_yuv();

// libavutil
int test_ffmpeg_libavutil_avdictionary(); // AVDictionary
int test_ffmpeg_libavutil_xtea(); // XTEA(eXtended Tiny Encryption Algorithm)
int test_ffmpeg_libavutil_twofish(); // Twofish crypto algorithm
int test_ffmpeg_libavutil_tea(); // TEA(Tiny Encryption Algorithm)
int test_ffmpeg_libavutil_sha512(); // SHA(Secure Hash Algorithm), SHA-512/224, SHA-512/256, SHA-384, SHA-512 
int test_ffmpeg_libavutil_sha(); // SHA(Secure Hash Algorithm), SHA-1, SHA-224, SHA-256 
int test_ffmpeg_libavutil_md5(); // MD5
int test_ffmpeg_libavutil_log(); // Log
int test_ffmpeg_libavutil_hash(); // hash function
int test_ffmpeg_libavutil_des(); // DES symmetric encryption algorithm
int test_ffmpeg_libavutil_aes(); // AES symmetric encryption algorithm
int test_ffmpeg_libavutil_base64(); // base64 codec

/////////////////////////// LIVE555 /////////////////////////////
int test_live555_rtsp_client();

/////////////////////////// V4L2 ///////////////////////////////
int test_v4l2_usb_stream();
int test_v4l2_get_video_device_info();
int test_v4l2_get_device_list(std::map<std::string, std::string>& device_list);
int test_v4l2_get_codec_type_list(const std::string& device_name, std::vector<int>& codec_list);
int test_v4l2_get_video_size_list(const std::string& device_name, int codec_type, std::vector<std::string>& size_list);

int test_get_usb_camera_vid_pid(); // vendor id, product id
int test_get_windows_camera_list();

/////////////////////////// libusb /////////////////////////////
int test_libusb_get_devices_list();
int test_libusb_hotplug();

#endif // FBC_FFMPEG_TEST_FUNSET_HPP_


#ifndef FBC_FFMPEG_TEST_FUNSET_HPP_
#define FBC_FFMPEG_TEST_FUNSET_HPP_

/////////////////////////// FFmpeg /////////////////////////////
// libavfilter

// libavdevice

// libavformat

// libavcodec

// libswresample
int test_ffmpeg_libswresample_resample(); // audio resample

// libswscale
int test_ffmpeg_libswscale_scale(); // image scale
int test_ffmpeg_libswscale_colorspace(); // color space convert

// libavutil
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

#endif // FBC_FFMPEG_TEST_FUNSET_HPP_


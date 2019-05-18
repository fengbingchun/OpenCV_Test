#include "funset.hpp"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif

// Blog: https://blog.csdn.net/fengbingchun/article/details/90313604

namespace {

int get_format_from_sample_fmt(const char** fmt, enum AVSampleFormat sample_fmt)
{
	struct sample_fmt_entry {
        	enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    	} sample_fmt_entries[] = {
        	{ AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        	{ AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        	{ AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        	{ AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        	{ AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    	};

	*fmt = nullptr;

    	for (int i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        	struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        	if (sample_fmt == entry->sample_fmt) {
            		*fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            		return 0;
        	}
    	}

    	fprintf(stderr, "Sample format %s not supported as output format\n", av_get_sample_fmt_name(sample_fmt));

	return AVERROR(EINVAL);
}

// Fill dst buffer with nb_samples, generated starting from t.
void fill_samples(double *dst, int nb_samples, int nb_channels, int sample_rate, double *t)
{
    	double tincr = 1.0 / sample_rate, *dstp = dst;
    	const double c = 2 * M_PI * 440.0;

    	// generate sin tone with 440Hz frequency and duplicated channels
    	for (int i = 0; i < nb_samples; i++) {
        	*dstp = sin(c * *t);
        	for (int j = 1; j < nb_channels; j++)
            		dstp[j] = dstp[0];
        
		dstp += nb_channels;
        	*t += tincr;
    	}
}

} // namespace

int test_ffmpeg_libswresample_resample()
{
	// reference: doc/examples/resample_audio.c

	fprintf(stdout, "swresample version: %d\n", swresample_version());
	fprintf(stdout, "swresample configuration: %s\n", swresample_configuration());
	fprintf(stdout, "swresample license: %s\n", swresample_license());	
	
	//create resampler context
	struct SwrContext* swr_ctx = swr_alloc();
	if (!swr_ctx) {
		fprintf(stderr, "fail to swr_alloc\n");
		return -1;
	}
	
	int64_t src_ch_layout = AV_CH_LAYOUT_STEREO, dst_ch_layout = AV_CH_LAYOUT_SURROUND;
	int src_rate = 48000, dst_rate = 44100;
	enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_DBL, dst_sample_fmt = AV_SAMPLE_FMT_S16;

	// set options
	av_opt_set_int(swr_ctx, "in_channel_layout",    src_ch_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate",       src_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", src_sample_fmt, 0);

	av_opt_set_int(swr_ctx, "out_channel_layout",    dst_ch_layout, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate",       dst_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

	// initialize the resampling context
	if (swr_init(swr_ctx) < 0) {
		fprintf(stderr, "fail to swr_init\n");
		return -1;
	}
	
	uint8_t **src_data = nullptr, **dst_data = nullptr;
	int src_nb_channels = 0, dst_nb_channels = 0;
	int src_linesize, dst_linesize;
	int src_nb_samples = 1024, dst_nb_samples, max_dst_nb_samples;
	
	// allocate source and destination samples buffers
	src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
	int ret = av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels, src_nb_samples, src_sample_fmt, 0);
	if (ret < 0) {
		fprintf(stderr, "fail to av_samples_alloc_array_and_samples\n");
		return -1;
	}

	// compute the number of converted samples: buffering is avoided
	// ensuring that the output buffer will contain at least all the converted input samples
	max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);

	// buffer is going to be directly written to a rawaudio file, no alignment
	dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
	ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 0);
	if (ret < 0) {
		fprintf(stderr, "fail to av_samples_alloc_array_and_samples\n");
		return -1;
	}

#ifdef _MSC_VER
	const char* file_name = "E:/GitCode/OpenCV_Test/test_images/xxx";
#else
	const char* file_name = "test_images/xxx";	
#endif
	FILE* dst_file = fopen(file_name, "wb");
	if (!dst_file) {
		fprintf(stderr, "fail to open file: %s\n", file_name);
		return -1;
	}

	double t = 0;
    	do {
        	// generate synthetic audio
        	fill_samples((double *)src_data[0], src_nb_samples, src_nb_channels, src_rate, &t);

        	// compute destination number of samples
        	dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) + src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
        	if (dst_nb_samples > max_dst_nb_samples) {
            		av_freep(&dst_data[0]);
            		ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 1);
            		if (ret < 0)
                		break;
            		max_dst_nb_samples = dst_nb_samples;
		}

        	// convert to destination format
        	ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)src_data, src_nb_samples);
        	if (ret < 0) {
            		fprintf(stderr, "fail to swr_convert\n");
            		return -1;
        	}
        	int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, ret, dst_sample_fmt, 1);
        	if (dst_bufsize < 0) {
           		fprintf(stderr, "fail to av_samples_get_buffer_size\n");
			return -1;
        	}
        	printf("t:%f in:%d out:%d\n", t, src_nb_samples, ret);
        	fwrite(dst_data[0], 1, dst_bufsize, dst_file);
    	} while (t < 10);

	const char* fmt = nullptr;
    	if ((ret = get_format_from_sample_fmt(&fmt, dst_sample_fmt)) < 0) {
        	fprintf(stderr, "fail to get_format_from_sample_fmt");
		return -1;
	}

    	fclose(dst_file);

    	if (src_data)
        	av_freep(&src_data[0]);
    	av_freep(&src_data);

    	if (dst_data)
        	av_freep(&dst_data[0]);
    	av_freep(&dst_data);

    	swr_free(&swr_ctx);

	return 0;
}


#include "funset.hpp"
#include <string.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/base64.h>
#include <libavutil/aes.h>
#include <libavutil/mem.h>
#include <libavutil/des.h>
#include <libavutil/hash.h>
#include <libavutil/log.h>
#include <libavutil/md5.h>
#include <libavutil/sha.h>
#include <libavutil/sha512.h>
#include <libavutil/tea.h>
#include <libavutil/xtea.h>
#include <libavutil/twofish.h>

#ifdef __cplusplus
}
#endif

// Blog: https://blog.csdn.net/fengbingchun/article/details/90313219

int test_ffmpeg_libavutil_xtea()
{
	const char* src = "https://blog.csdn.net/fengbingchun";
	const uint8_t key[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	
	size_t length = strlen(src);
	int blocks_count = (length + 7) / 8; // count number of 8 byte blocks
	std::unique_ptr<uint8_t[]> cipher_text(new uint8_t[blocks_count * 8]), plain_text(new uint8_t[blocks_count * 8]);
	fprintf(stdout, "src length: %d, blocks_count: %d\n", length, blocks_count);
	
	// encrypt
	AVXTEA* ctx1 = av_xtea_alloc();
	if (!ctx1) {
		fprintf(stderr, "fail to av_xtea_alloc\n");
		return -1;
	}
	
	av_xtea_init(ctx1, key);
	av_xtea_crypt(ctx1, cipher_text.get(), (const uint8_t*)src, blocks_count, nullptr, 0);
	av_free(ctx1);

	// decrypt
	AVXTEA* ctx2 = av_xtea_alloc();
	if (!ctx2) {
		fprintf(stderr, "fail to av_xtea_alloc\n");
		return -1;
	}
	
	av_xtea_init(ctx2, key);
	av_xtea_crypt(ctx2, plain_text.get(), (const uint8_t*)cipher_text.get(), blocks_count, nullptr, 1);
	av_free(ctx2);

	fprintf(stdout, "src data: %s\n", src);
	fprintf(stdout, "cipher text: %s\n", (const char*)cipher_text.get());
	fprintf(stdout, "plain text: %s\n", (const char*)plain_text.get());
	
	if (memcmp(src, (const char*)plain_text.get(), length)) {
		fprintf(stderr, "fail to xtea encrypt/decrypt\n");
		return -1;
	}	

	return 0;
}

int test_ffmpeg_libavutil_twofish()
{
	return -1;
	
	// the execution result is incorrect, i don't know why ???
	const char* src = "https://blog.csdn.net/fengbingchun";
	const uint8_t key[32] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	const int key_bits = 256; // 128, 192, 256
	size_t length = strlen(src);
	int blocks_count = (length + 15) / 16; // count number of 16 byte blocks
	std::unique_ptr<uint8_t[]> cipher_text(new uint8_t[blocks_count * 16]), plain_text(new uint8_t[blocks_count * 16]);
	fprintf(stdout, "src length: %d, blocks_count: %d\n", length, blocks_count);
	uint8_t iv[16];
	memcpy(iv, "HALLO123HALLO123", 16);
	
	// encrypt
	AVTWOFISH* ctx1 = av_twofish_alloc();
	if (!ctx1) {
		fprintf(stderr, "fail to av_twofish_alloc\n");
		return -1;
	}
	
	av_twofish_init(ctx1, key, key_bits);
	av_twofish_crypt(ctx1, cipher_text.get(), (const uint8_t*)src, blocks_count, /*iv*/nullptr, 0);
	av_free(ctx1);

	if (blocks_count * 16 > length)
		cipher_text[length] = '\0';

	// decrypt
	AVTWOFISH* ctx2 = av_twofish_alloc();
	if (!ctx2) {
		fprintf(stderr, "fail to av_twofish_alloc\n");
		return -1;
	}
	
	av_twofish_init(ctx2, key, key_bits);
	av_twofish_crypt(ctx2, plain_text.get(), (uint8_t*)cipher_text.get(), blocks_count, /*iv*/nullptr, 1);
	av_free(ctx2);

	if (blocks_count * 16 > length)
		plain_text[length] = '\0';

	fprintf(stdout, "src data: %s\n", src);
	fprintf(stdout, "cipher text: %s\n", (const char*)cipher_text.get());
	fprintf(stdout, "plain text: %s\n", (const char*)plain_text.get());
	
	if (memcmp(src, (const char*)plain_text.get(), length)) {
		fprintf(stderr, "fail to twofish encrypt/decrypt\n");
		return -1;
	}	

	return 0;
}

int test_ffmpeg_libavutil_tea()
{
	const char* src = "https://blog.csdn.net/fengbingchun";
	const uint8_t key[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	
	size_t length = strlen(src);
	int blocks_count = (length + 7) / 8; // count number of 8 byte blocks
	std::unique_ptr<uint8_t[]> cipher_text(new uint8_t[blocks_count * 8]), plain_text(new uint8_t[blocks_count * 8]);
	fprintf(stdout, "src length: %d, blocks_count: %d\n", length, blocks_count);
	
	// encrypt
	struct AVTEA* ctx1 = av_tea_alloc();
	if (!ctx1) {
		fprintf(stderr, "fail to av_tea_alloc\n");
		return -1;
	}
	
	av_tea_init(ctx1, key, 64);
	av_tea_crypt(ctx1, cipher_text.get(), (const uint8_t*)src, blocks_count, nullptr, 0);
	av_free(ctx1);

	// decrypt
	struct AVTEA* ctx2 = av_tea_alloc();
	if (!ctx2) {
		fprintf(stderr, "fail to av_tea_alloc\n");
		return -1;
	}
	
	av_tea_init(ctx2, key, 64);
	av_tea_crypt(ctx2, plain_text.get(), (const uint8_t*)cipher_text.get(), blocks_count, nullptr, 1);
	av_free(ctx2);

	fprintf(stdout, "src data: %s\n", src);
	fprintf(stdout, "cipher text: %s\n", (const char*)cipher_text.get());
	fprintf(stdout, "plain text: %s\n", (const char*)plain_text.get());
	
	if (memcmp(src, (const char*)plain_text.get(), length)) {
		fprintf(stderr, "fail to tea encrypt/decrypt\n");
		return -1;
	}	
	
	return 0;
}

int test_ffmpeg_libavutil_sha512()
{
	const int hash_length[4] = {224, 256, 384, 512}; // SHA-512/224, SHA-512/256, SHA-384, SHA-512
	const char* src = "https://blog.csdn.net/fengbingchun";

	struct AVSHA512* ctx = av_sha512_alloc();
	if (!ctx) {
		fprintf(stderr, "fail to av_sha512_alloc\n");
		return -1;
	}

	for (int i = 0; i < 4; ++i) {
		unsigned char digest[64];

		av_sha512_init(ctx, hash_length[i]);
		av_sha512_update(ctx, (const uint8_t*)src, strlen(src));
		av_sha512_final(ctx, digest);				
	
		fprintf(stdout, "sha512: ");
		//for (int j = 0; j < 64; ++j)
		for (int j = 0; j < hash_length[i] >> 3; ++j)
			fprintf(stdout, "%02x", digest[j]);
		fprintf(stdout, "\n");
	}

	av_free(ctx);
	
	return 0;
}

int test_ffmpeg_libavutil_sha()
{
	const int hash_length[3] = {160, 224, 256}; // SHA-1, SHA-224, SHA-256
	const char* src = "https://blog.csdn.net/fengbingchun";

	struct AVSHA* ctx = av_sha_alloc();
	if (!ctx) {
		fprintf(stderr, "fail to av_sha_alloc\n");
		return -1;
	}

	for (int i = 0; i < 3; ++i) {
		unsigned char digest[32];

		av_sha_init(ctx, hash_length[i]);
		av_sha_update(ctx, (const uint8_t*)src, strlen(src));
		av_sha_final(ctx, digest);				
	
		fprintf(stdout, "sha: ");
		//for (int j = 0; j < 32; ++j)
		for (int j = 0; j < hash_length[i] >> 3; ++j)
			fprintf(stdout, "%02x", digest[j]);
		fprintf(stdout, "\n");
	}

	av_free(ctx);
	
	return 0;
}

int test_ffmpeg_libavutil_md5()
{
	const char* src = "https://blog.csdn.net/fengbingchun";
	uint8_t md5val[16];

	av_md5_sum(md5val, (const uint8_t*)src, strlen(src));

	fprintf(stdout, "md5sum: ");
	for (int i = 0; i < 16; ++i)
		fprintf(stdout, "%02x", md5val[i]);
	fprintf(stdout, "\n");

	return 0;
}

int test_ffmpeg_libavutil_log()
{
	av_log_set_level(AV_LOG_TRACE);

	av_log(nullptr, AV_LOG_TRACE, "trace message\n");
	av_log(nullptr, AV_LOG_DEBUG, "debug message\n");
	av_log(nullptr, AV_LOG_VERBOSE, "verbose message\n");
	av_log(nullptr, AV_LOG_INFO, "info message\n");
	av_log(nullptr, AV_LOG_WARNING, "warning message\n");
	av_log(nullptr, AV_LOG_ERROR, "error message\n");
	av_log(nullptr, AV_LOG_FATAL, "fatal message\n");
	av_log(nullptr, AV_LOG_PANIC, "panic message\n");

	return 0;
}

int test_ffmpeg_libavutil_hash()
{
	std::vector<const char*> hash_names;
	int index = 0;

	do {
		hash_names.push_back(av_hash_names(index));
		++index;
	} while(hash_names[hash_names.size()-1]);

	fprintf(stdout, "supported hash algorithms: ");
	for (auto name : hash_names) {
		if (name)
			fprintf(stdout, "%s,  ", name);
	}
	fprintf(stdout, "\n");

	const char* src[2] = {"https://blog.csdn.net/fengbingchun", "https://github.com/fengbingchun"};
	int dst_buf_size = AV_HASH_MAX_SIZE * 8;

	for (int i = 0; i < 2; ++i) {
		struct AVHashContext* ctx = nullptr;
		if (av_hash_alloc(&ctx, hash_names[i]) < 0) {
			fprintf(stderr, "fail to av_hash_alloc: %d\n", i);
			return -1;
		}

		av_hash_init(ctx);
	
		av_hash_update(ctx, (const uint8_t*)src[i], strlen(src[i]));

		std::unique_ptr<uint8_t[]> dst(new uint8_t[dst_buf_size]);
		memset(dst.get(), 0, dst_buf_size);

		av_hash_final_hex(ctx, dst.get(), dst_buf_size);
		fprintf(stdout, "hash name: %s, hex: %s\n", av_hash_get_name(ctx), dst.get());

		av_hash_freep(&ctx);
	}

	return 0;
}

int test_ffmpeg_libavutil_des()
{
	const uint8_t key[16] = { 0x10, 0xa5, 0x88, 0x69, 0xd7, 0x4b, 0xe5, 0xa3, 0x74, 0xcf, 0x86, 0x7c, 0xfb, 0x47, 0x38, 0x59 };
	const std::string src = "https://blog.csdn.net/fengbingchun";
	size_t length = src.length();
	int blocks_count = (length + 7) / 8; // count number of 8 byte blocks
	std::unique_ptr<uint8_t[]> cipher_text(new uint8_t[blocks_count * 8]), plain_text(new uint8_t[blocks_count * 8]);
	fprintf(stdout, "src length: %d, blocks_count: %d\n", length, blocks_count);

	// encryption
	struct AVDES b1;
	av_des_init(&b1, key, 64, 0);
	av_des_crypt(&b1, cipher_text.get(), (const uint8_t*)src.c_str(), blocks_count, nullptr, 0);
	fprintf(stdout, "src data: %s\n", src.c_str());
	fprintf(stdout, "cipher text: %s\n", (const char*)cipher_text.get());

	// decryption
	struct AVDES b2;
	av_des_init(&b2, key, 64, 1);
	av_des_crypt(&b2, plain_text.get(), (const uint8_t*)cipher_text.get(), blocks_count, nullptr, 1);
	fprintf(stdout, "plain text: %s\n", (const char*)plain_text.get());

	if (memcmp(src.c_str(), (const char*)plain_text.get(), length)) {
		fprintf(stderr, "fail to des encrypt/decrypt\n");
		return -1;
	
	}

	return 0;
}

int test_ffmpeg_libavutil_aes()
{
	const uint8_t key[16] = { 0x10, 0xa5, 0x88, 0x69, 0xd7, 0x4b, 0xe5, 0xa3, 0x74, 0xcf, 0x86, 0x7c, 0xfb, 0x47, 0x38, 0x59 };
	const std::string src = "https://blog.csdn.net/fengbingchun";
	size_t length = src.length();
	int blocks_count = (length + 15) / 16; // count number of 16 byte blocks
	std::unique_ptr<uint8_t[]> cipher_text(new uint8_t[blocks_count * 16]), plain_text(new uint8_t[blocks_count * 16]);
	fprintf(stdout, "src length: %d, blocks_count: %d\n", length, blocks_count);

	// encryption
	struct AVAES* b1 = av_aes_alloc();
	if (!b1) {
		fprintf(stderr, "fail to av_aes_alloc\n");
		return -1;
	}
	av_aes_init(b1, key, 128, 0);
	av_aes_crypt(b1, cipher_text.get(), (const uint8_t*)src.c_str(), blocks_count, nullptr, 0);
	av_free(b1);
	fprintf(stdout, "src data: %s\n", src.c_str());
	fprintf(stdout, "cipher text: %s\n", (const char*)cipher_text.get());

	// decryption
	struct AVAES* b2 = av_aes_alloc();
	if (!b2) {
		fprintf(stderr, "fail to av_aes_alloc\n");
		return -1;
	}
	av_aes_init(b2, key, 128, 1);
	av_aes_crypt(b2, plain_text.get(), (const uint8_t*)cipher_text.get(), blocks_count, nullptr, 1);
	av_free(b2);
	fprintf(stdout, "plain text: %s\n", (const char*)plain_text.get());

	if (memcmp(src.c_str(), (const char*)plain_text.get(), length)) {
		fprintf(stderr, "fail to aes encrypt/decrypt\n");
		return -1;
	
	}

	return 0;
}

int test_ffmpeg_libavutil_base64()
{
	const std::string blog_addr = "https://blog.csdn.net/fengbingchun";
	int length_src = blog_addr.length();
	int length_dst = AV_BASE64_SIZE(length_src);
	std::unique_ptr<char[]> dst1(new char[length_dst]);

	char* tmp = av_base64_encode(dst1.get(), length_dst, (const uint8_t*)blog_addr.c_str(), length_src);
	if (!tmp) {
		fprintf(stderr, "fail to av_base64_encode\n");
		return -1;
	}

	int length_dst2 = AV_BASE64_DECODE_SIZE(length_dst);
	std::unique_ptr<uint8_t[]> dst2(new uint8_t[length_dst2]);
	int size = av_base64_decode(dst2.get(), dst1.get(), length_dst2);
	if (size < 0) {
		fprintf(stderr, "fail to av_base64_decode");
		return -1;
	}

	if (memcmp(blog_addr.c_str(), (const char*)dst2.get(), size)) {
		fprintf(stderr, "fail to base64 encode/decode\n");
		return -1;
	}

	if (length_dst2 > size)
		dst2.get()[size] = '\0';

	fprintf(stdout, "src string: %s\n", blog_addr.c_str());
	fprintf(stdout, "encode result: %s\n", dst1.get());
	fprintf(stdout, "decode result: %s\n", (char*)dst2.get());

	return 0;
}



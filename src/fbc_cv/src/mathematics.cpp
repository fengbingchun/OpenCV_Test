// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <limits.h>
#include <stdio.h>
#include "mathematics.hpp"

// reference: ffmpeg 4.2
//            libavutil/mathematics.c

#ifdef _MSC_VER

namespace fbc {

/* Stein's binary GCD algorithm:
* https://en.wikipedia.org/wiki/Binary_GCD_algorithm */
int64_t av_gcd(int64_t a, int64_t b) {
	int za, zb, k;
	int64_t u, v;
	if (a == 0)
		return b;
	if (b == 0)
		return a;
	za = ff_ctzll(a);
	zb = ff_ctzll(b);
	k = FFMIN(za, zb);
	u = llabs(a >> za);
	v = llabs(b >> zb);
	while (u != v) {
		if (u > v)
			FFSWAP(int64_t, v, u);
		v -= u;
		v >>= ff_ctzll(v);
	}
	return (uint64_t)u << k;
}

int64_t av_rescale(int64_t a, int64_t b, int64_t c)
{
	return av_rescale_rnd(a, b, c, AV_ROUND_NEAR_INF);
}

int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd)
{
	int64_t r = 0;
	av_assert2(c > 0);
	av_assert2(b >= 0);
	av_assert2((unsigned)(rnd&~AV_ROUND_PASS_MINMAX) <= 5 && (rnd&~AV_ROUND_PASS_MINMAX) != 4);

	if (c <= 0 || b < 0 || !((unsigned)(rnd&~AV_ROUND_PASS_MINMAX) <= 5 && (rnd&~AV_ROUND_PASS_MINMAX) != 4))
		return INT64_MIN;

	if (rnd & AV_ROUND_PASS_MINMAX) {
		if (a == INT64_MIN || a == INT64_MAX)
			return a;
		rnd = static_cast<AVRounding>(rnd - AV_ROUND_PASS_MINMAX);
	}

	if (a < 0) {
		ERROR_POS
		return -(av_rescale_rnd(-FFMAX(a, -INT64_MAX), b, c, static_cast<AVRounding>(rnd ^ ((rnd >> 1)) & 1))); // differenct
	}


	if (rnd == AV_ROUND_NEAR_INF)
		r = c / 2;
	else if (rnd & 1)
		r = c - 1;

	if (b <= INT_MAX && c <= INT_MAX) {
		if (a <= INT_MAX)
			return (a * b + r) / c;
		else {
			int64_t ad = a / c;
			int64_t a2 = (a % c * b + r) / c;
			if (ad >= INT32_MAX && b && ad > (INT64_MAX - a2) / b)
				return INT64_MIN;
			return ad * b + a2;
		}
	} else {
#if 1
		uint64_t a0 = a & 0xFFFFFFFF;
		uint64_t a1 = a >> 32;
		uint64_t b0 = b & 0xFFFFFFFF;
		uint64_t b1 = b >> 32;
		uint64_t t1 = a0 * b1 + a1 * b0;
		uint64_t t1a = t1 << 32;
		int i;

		a0 = a0 * b0 + t1a;
		a1 = a1 * b1 + (t1 >> 32) + (a0 < t1a);
		a0 += r;
		a1 += a0 < r;

		for (i = 63; i >= 0; i--) {
			a1 += a1 + ((a0 >> i) & 1);
			t1 += t1;
			if (c <= a1) {
				a1 -= c;
				t1++;
			}
		}
		if (t1 > INT64_MAX)
			return INT64_MIN;
		return t1;
#else
		/* reference code doing (a*b + r) / c, requires libavutil/integer.h */
		AVInteger ai;
		ai = av_mul_i(av_int2i(a), av_int2i(b));
		ai = av_add_i(ai, av_int2i(r));

		return av_i2int(av_div_i(ai, av_int2i(c)));
#endif
	}
}

int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq, enum AVRounding rnd)
{
	int64_t b = bq.num * (int64_t)cq.den;
	int64_t c = cq.num * (int64_t)bq.den;
	return av_rescale_rnd(a, b, c, rnd);
}

int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)
{
	return av_rescale_q_rnd(a, bq, cq, AV_ROUND_NEAR_INF);
}

int64_t av_add_stable(AVRational ts_tb, int64_t ts, AVRational inc_tb, int64_t inc)
{
	int64_t m, d;

	if (inc != 1)
		inc_tb = av_mul_q(inc_tb, AVRational{ inc, 1 });

	m = inc_tb.num * (int64_t)ts_tb.den;
	d = inc_tb.den * (int64_t)ts_tb.num;

	if (m % d == 0 && ts <= INT64_MAX - m / d)
		return ts + m / d;
	if (m < d)
		return ts;

	{
		int64_t old = av_rescale_q(ts, ts_tb, inc_tb);
		int64_t old_ts = av_rescale_q(old, inc_tb, ts_tb);

		if (old == INT64_MAX)
			return ts;

		return av_rescale_q(old + 1, inc_tb, ts_tb) + (ts - old_ts);
	}
}

} // namespace fbc

#endif // _MSC_VER

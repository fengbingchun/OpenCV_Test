// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <limits>
#include "avrational.hpp"
#include "mathematics.hpp"

// reference: ffmpeg 4.2
//            libavutil/rational.c

#ifdef _MSC_VER

namespace fbc {

AVRational av_mul_q(AVRational b, AVRational c)
{
	av_reduce(&b.num, &b.den,
		b.num * (int64_t)c.num,
		b.den * (int64_t)c.den, INT_MAX);
	return b;
}

int av_reduce(int *dst_num, int *dst_den,
	int64_t num, int64_t den, int64_t max)
{
	AVRational a0 = { 0, 1 }, a1 = { 1, 0 };
	int sign = (num < 0) ^ (den < 0);
	int64_t gcd = av_gcd(FFABS(num), FFABS(den));

	if (gcd) {
		num = FFABS(num) / gcd;
		den = FFABS(den) / gcd;
	}
	if (num <= max && den <= max) {
		a1 = AVRational{ num, den };
		den = 0;
	}

	while (den) {
		uint64_t x = num / den;
		int64_t next_den = num - den * x;
		int64_t a2n = x * a1.num + a0.num;
		int64_t a2d = x * a1.den + a0.den;

		if (a2n > max || a2d > max) {
			if (a1.num) x = (max - a0.num) / a1.num;
			if (a1.den) x = FFMIN(x, (max - a0.den) / a1.den);

			if (den * (2 * x * a1.den + a0.den) > num * a1.den)
				a1 = AVRational{ x * a1.num + a0.num, x * a1.den + a0.den };
			break;
		}

		a0 = a1;
		a1 = AVRational{ a2n, a2d };
		num = den;
		den = next_den;
	}
	av_assert2(av_gcd(a1.num, a1.den) <= 1U);
	av_assert2(a1.num <= max && a1.den <= max);

	*dst_num = sign ? -a1.num : a1.num;
	*dst_den = a1.den;

	return den == 0;
}

AVRational av_d2q(double d, int max)
{
	AVRational a;
	int exponent;
	int64_t den;
	if (isnan(d))
		return AVRational{ 0, 0 };
	if (fabs(d) > INT_MAX + 3LL)
		return AVRational{ d < 0 ? -1 : 1, 0 };
	frexp(d, &exponent);
	exponent = FFMAX(exponent - 1, 0);
	den = 1LL << (61 - exponent);
	// (int64_t)rint() and llrint() do not work with gcc on ia64 and sparc64,
	// see Ticket2713 for affected gcc/glibc versions
	av_reduce(&a.num, &a.den, floor(d * den + 0.5), den, max);
	if ((!a.num || !a.den) && d && max>0 && max<INT_MAX)
		av_reduce(&a.num, &a.den, floor(d * den + 0.5), den, INT_MAX);

	return a;
}

} // namespace fbc

#endif // _MSC_VER

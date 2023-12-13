//
// Created by Frank on 2023/12/6.
//

#ifndef XD_RT_FLOATWITHERROR_H
#define XD_RT_FLOATWITHERROR_H
#include <cmath>
#include <numeric>

#include "MathUtil.h"

namespace xd {
class FloatWithError {
public:
	inline static FloatWithError fromFloatError(float f, float err)
	{
		err = std::fabs(err);
		return {f - err, f + err};
	}
	FloatWithError() = default;
	FloatWithError(const FloatWithError& other) = default;
	FloatWithError(FloatWithError&& other) noexcept = default;
	FloatWithError& operator=(const FloatWithError& other) = default;
	FloatWithError& operator=(FloatWithError&& other) noexcept = default;
	explicit FloatWithError(float f) : low(f), high(f) {}
	FloatWithError(float low, float high) : low(low), high(high) {}

	float middle() const { return (low + high) / 2.f; }
	float extent() const { return high - low; }
	bool isExact() const { return low == high; }
	operator float() const { return middle(); }
	bool has(float f) const { return f >= low && f <= high; }
	friend FloatWithError operator+(const FloatWithError& lhs, const FloatWithError& rhs)
	{
		return {nextFloatDown(lhs.low + rhs.low), nextFloatUp(lhs.high + rhs.high)};
	}

	friend FloatWithError operator+(const FloatWithError& lhs, float rhs)
	{
		return lhs + FloatWithError{rhs};
	}

	friend FloatWithError operator+(float lhs, const FloatWithError& rhs)
	{
		return FloatWithError{lhs} + rhs;
	}

	FloatWithError& operator+=(const FloatWithError& rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	FloatWithError& operator+=(float rhs) { return *this += FloatWithError{rhs}; }

	friend FloatWithError operator-(const FloatWithError& op) { return {-op.high, -op.low}; }

	friend FloatWithError operator-(const FloatWithError& lhs, const FloatWithError& rhs)
	{
		FloatWithError res;
		res.low = nextFloatDown(lhs.low - rhs.low);
		res.high = nextFloatUp(lhs.high - rhs.high);
		return res;
	}

	friend FloatWithError operator-(const FloatWithError& lhs, float rhs)
	{
		return lhs - FloatWithError{rhs};
	}

	friend FloatWithError operator-(float lhs, const FloatWithError& rhs)
	{
		return FloatWithError{lhs} - rhs;
	}

	FloatWithError& operator-=(const FloatWithError& rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	FloatWithError& operator-=(float rhs) { return *this -= FloatWithError{rhs}; }

	friend FloatWithError operator*(const FloatWithError& lhs, const FloatWithError& rhs)
	{
		std::array<float, 4> products{lhs.low * rhs.low, lhs.low * rhs.high, lhs.high * rhs.low,
									  lhs.high * rhs.high};
		std::ranges::sort(products);
		return {nextFloatDown(products.front()), nextFloatUp(products.back())};
	}

	friend FloatWithError operator*(const FloatWithError& lhs, float rhs)
	{
		return lhs * FloatWithError{rhs};
	}

	friend FloatWithError operator*(float lhs, const FloatWithError& rhs)
	{
		return FloatWithError{lhs} * rhs;
	}

	friend FloatWithError operator/(const FloatWithError& lhs, const FloatWithError& rhs)
	{
		constexpr float inf = std::numeric_limits<float>::infinity();
		if (rhs.has(0))
			// The interval we're dividing by straddles zero, so just return an interval of
			// everything.
			return FloatWithError(-inf, inf);

		std::array<float, 4> quotients{lhs.low / rhs.low, lhs.low / rhs.high, lhs.high / rhs.low,
									   lhs.high / rhs.high};
		std::ranges::sort(quotients);
		return {nextFloatDown(quotients.front()), nextFloatUp(quotients.back())};
	}

	friend FloatWithError operator/(const FloatWithError& lhs, float rhs)
	{
		return lhs / FloatWithError{rhs};
	}

	friend FloatWithError operator/(float lhs, const FloatWithError& rhs)
	{
		return FloatWithError{lhs} / rhs;
	}

	friend FloatWithError sqr(const FloatWithError& op)
	{
		auto low = op.low * op.low;
		auto high = op.high * op.high;
		if (low > high)
			std::swap(low, high);
		if (op.has(0)) {
			return {0, high};
		}
		return {low, high};
	}
	friend FloatWithError sqrt(const FloatWithError& op)
	{
		auto high = nextFloatUp(std::sqrtf(op.high));
		if (op.has(0)) {
			return {0, high};
		}
		return {nextFloatDown(std::sqrtf(op.low)), high};
	}

	friend FloatWithError abs(const FloatWithError& op)
	{
		if (op.low >= 0)
			return op;
		else if (op.high <= 0)
			return -op;
		else
			return {0.f, std::max(-op.low, op.high)};
	}
	float low, high;
};

inline uint32_t solveQuadraticRealWithError(const FloatWithError& a,
											const FloatWithError& b,
											const FloatWithError& c,
											FloatWithError& x1,
											FloatWithError& x2)
{
	const auto delta = b * b - 4.f * a * c;
	constexpr float eps = 1e-6;
	if (delta.low < 0.f) {
		return 0;
	}
	else {
		x1 = (-b + sqrt(delta)) / (2.f * a);
		x2 = (-b - sqrt(delta)) / (2.f * a);
		if (x1 > x2)
			std::swap(x1, x2);
		return 2;
	}
}
}  // namespace xd
#endif	// XD_RT_FLOATWITHERROR_H

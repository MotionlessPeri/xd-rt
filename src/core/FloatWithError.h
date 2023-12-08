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

protected:
	float low, high;
};
}  // namespace xd
#endif	// XD_RT_FLOATWITHERROR_H

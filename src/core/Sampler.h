//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_SAMPLER_H
#define XD_RT_SAMPLER_H

#include <span>
#include <vector>
#include "MathTypes.h"
namespace xd {
class Sampler {
public:
	explicit Sampler(int samplePerPixel) : samplePerPixel(samplePerPixel) {}
	virtual ~Sampler() = default;
	virtual void setCurrentPixel(const Vector2i& pixel)
	{
		currentPixel = pixel;
		currentSampleIndex = 0;
	}
	virtual void request1DArray(int n) = 0;
	virtual void request2DArray(int n) = 0;
	/**
	 * get previous requested 1d sample array
	 * @param n the size of previous requested array
	 * @return a span describes the sample array. If get sample array fails, the span will be empty
	 */
	virtual const std::span<const float> get1DArray(int n) = 0;
	/**
	 * get previous requested 2d sample array
	 * @param n the size of previous requested array
	 * @return a span describes the sample array. If get sample array fails, the span will be empty
	 */
	virtual const std::span<const Vector2f> get2DArray(int n) = 0;
	virtual float sample1D() = 0;
	virtual Vector2f sample2D() = 0;
	virtual bool startNextSample()
	{
		++currentSampleIndex;
		return currentSampleIndex < samplePerPixel;
	}
	virtual std::unique_ptr<Sampler> clone(int seed) const = 0;

protected:
	int samplePerPixel;
	int currentSampleIndex = 0;
	Vector2i currentPixel;
};

}  // namespace xd

#endif	// XD_RT_SAMPLER_H

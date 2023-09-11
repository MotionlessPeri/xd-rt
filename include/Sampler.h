//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_SAMPLER_H
#define XD_RT_SAMPLER_H

#include <vector>
#include "MathType.h"
namespace xd {
class Sampler {
public:
	virtual std::vector<Vector2f> generateSamples() = 0;
};

class SimpleSampler : public Sampler {
public:
	SimpleSampler(uint32_t width, uint32_t height);

	std::vector<Vector2f> generateSamples() override;

protected:
	uint32_t width;
	uint32_t height;
};

}  // namespace xd

#endif	// XD_RT_SAMPLER_H

//
// Created by Frank on 2023/12/26.
//

#ifndef XD_RT_LIGHTSAMPLER_H
#define XD_RT_LIGHTSAMPLER_H
#include "CoreTypes.h"
namespace xd {

class LightSampler {
public:
	virtual ~LightSampler() = default;
	struct SampleLightResult {
		std::shared_ptr<Light> light;
		float pmf;
	};
	virtual SampleLightResult sampleLight(float uSample, const LocalGeomParams& geom) const = 0;
	virtual float getPmf(const LocalGeomParams& geom,
						 const std::shared_ptr<Light>& light) const = 0;
};

class SimpleLightSampler : public LightSampler {
public:
	explicit SimpleLightSampler(const Scene& scene);
	explicit SimpleLightSampler(std::vector<std::shared_ptr<Light>> lights);
	SampleLightResult sampleLight(float uSample, const LocalGeomParams& geom) const override;
	float getPmf(const LocalGeomParams& geom, const std::shared_ptr<Light>& light) const override;

protected:
	std::vector<std::shared_ptr<Light>> lights;
};
}  // namespace xd

#endif	// XD_RT_LIGHTSAMPLER_H

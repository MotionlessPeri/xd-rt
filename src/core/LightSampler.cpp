//
// Created by Frank on 2023/12/26.
//

#include "LightSampler.h"
#include "Light.h"
#include "Scene.h"
using namespace xd;
SimpleLightSampler::SimpleLightSampler(const Scene& scene) : SimpleLightSampler(scene.getLights())
{
}
SimpleLightSampler::SimpleLightSampler(std::vector<std::shared_ptr<Light>> lights)
	: lights(std::move(lights))
{
}
LightSampler::SampleLightResult SimpleLightSampler::sampleLight(float uSample,
																const LocalGeomParams& geom) const
{
	SampleLightResult ret;
	const auto lightSize = lights.size();
	ret.light = lights[std::size_t(uSample * lightSize)];
	ret.pmf = 1.f / lightSize;
	return ret;
}
float SimpleLightSampler::getPmf(const LocalGeomParams& geom,
								 const std::shared_ptr<Light>& light) const
{
	const auto lightSize = lights.size();
	return 1.f / lightSize;
}

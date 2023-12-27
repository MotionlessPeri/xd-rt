//
// Created by Frank on 2023/11/28.
//
#include "DirectIntegrator.h"
#include "Camera.h"
#include "Light.h"
#include "LightSampler.h"
#include "Sampler.h"
#include "Scene.h"
using namespace xd;
MIDirectIntegrator::MIDirectIntegrator(const std::shared_ptr<Sampler>& sampler)
	: SamplerIntegrator(sampler)
{
}

MIDirectIntegrator::MIDirectIntegrator(const IntegratorConfig& config,
									   const std::shared_ptr<Sampler>& sampler)
	: SamplerIntegrator(config, sampler)
{
}

void MIDirectIntegrator::render(const Scene& scene)
{
	const auto& lights = scene.getLights();
	for (const auto& light : lights) {
		const auto numSamples = light->getNumSamples();
		sampler->request2DArray(numSamples);  // for sampling light
		sampler->request2DArray(numSamples);  // for sampling bsdf
	}
	SamplerIntegrator::render(scene);
}

ColorRGB MIDirectIntegrator::Li(const Ray& ray, const Scene& scene, Sampler& sampler)
{
	Vector3f Li{0, 0, 0};
	HitRecord primRec;
	if (!scene.hit(ray, primRec)) {
		const auto env = scene.getEnvironment();
		if (env) {
			Li = env->getRadiance({}, ray.d);
		}
		return Li;
	}

	const auto& lights = scene.getLights();
	for (auto i : std::views::iota(0u, lights.size())) {
		const auto& light = lights[i];
		const uint32_t numLightSamples = light->getNumSamples();
		const auto uLights = sampler.get2DArray(numLightSamples);
		const auto uBxdfs = sampler.get2DArray(numLightSamples);
		Vector3f LiThisLight{0, 0, 0};
		for (auto sampleIdx : std::views::iota(0u, numLightSamples)) {
			Vector2f uLight, uBxdf;
			if (uLights.empty() || uBxdfs.empty()) {
				uLight = sampler.sample2D();
				uBxdf = sampler.sample2D();
			}
			else [[likely]] {
				uLight = uLights[sampleIdx];
				uBxdf = uBxdfs[sampleIdx];
			}
			LiThisLight +=
				EstimateDirect(ray, primRec, uLight, *light, uBxdf, 0.f, scene, nullptr, true);
		}
		Li += LiThisLight / light->getNumSamples();
	}
	return Li;
}
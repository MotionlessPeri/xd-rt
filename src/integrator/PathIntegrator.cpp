//
// Created by Frank on 2023/12/1.
//
#include <chrono>
#include "Integrator.h"
#include "Light.h"
#include "Primitive.h"
#include "Ray.h"
#include "Sampler.h"
#include "Scene.h"
using namespace xd;
PathIntegrator::PathIntegrator(const std::shared_ptr<Sampler>& sampler, int maxDepth)
	: SamplerIntegrator(sampler), maxDepth(maxDepth)
{
}
ColorRGB PathIntegrator::Li(const Ray& r, const Scene& scene, Sampler& sampler)
{
	auto ray = r;
	ColorRGB Li{0, 0, 0};
	Vector3f weight{1, 1, 1};
	const auto& lights = scene.getLights();
	UniformDiscreteDistribution<1> lightDis(
		0, lights.size() - 1, (int)std::chrono::system_clock::now().time_since_epoch().count());
	for (auto depth : std::views::iota(0, maxDepth)) {
		HitRecord primRec;
		if (!scene.hit(ray, primRec)) {
			const auto env = scene.getEnvironment();
			if (env) {
				Li += weight.cwiseProduct(env->getRadiance(primRec, ray.d));
			}
			break;
		}

		// TODO: add processing hit a light
		const auto material = primRec.primitive->getMaterial();
		const auto lightIdx = lightDis.sample();
		const auto lightPdf = 1.f / lights.size();
		const auto& light = lights[lightIdx];
		Li += weight.cwiseProduct(EstimateDirect(ray, primRec, sampler.sample2D(), *light,
												 sampler.sample2D(), scene)) /
			  lightPdf;

		float newWiPdf;
		const auto newWi =
			material->sampleDirectionWithPdf(sampler.sample2D(), primRec, -ray.d, newWiPdf);
		ray = primRec.spawnRay(newWi);
		const auto brdfVal = material->getBRDF(primRec, -ray.d, newWi);
		weight = weight.cwiseProduct(brdfVal * std::fabs(newWi.dot(primRec.n)) / newWiPdf);

		constexpr int minDepth = 3;
		if (depth >= minDepth) {
			const auto q = std::max(0.05f, weight.maxCoeff());
			if (sampler.sample1D() < q)
				break;
			weight /= 1 - q;
		}
	}

	return Li;
}

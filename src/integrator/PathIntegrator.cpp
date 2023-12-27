//
// Created by Frank on 2023/12/1.
//
#include "PathIntegrator.h"
#include <chrono>
#include "Light.h"
#include "LightSampler.h"
#include "Ray.h"
#include "Sampler.h"
#include "Scene.h"
#include "distribution/DiscreteUniformDistribution.h"

#define DEBUG_LAST_PATH_CONTRIB_ONLY 0
using namespace xd;
PathIntegrator::PathIntegrator(const std::shared_ptr<Sampler>& sampler, int maxDepth)
	: SamplerIntegrator(sampler), maxDepth(maxDepth)
{
}

PathIntegrator::PathIntegrator(const IntegratorConfig& config,
							   const std::shared_ptr<Sampler>& sampler,
							   int max_depth)
	: SamplerIntegrator(config, sampler), maxDepth(max_depth)
{
}

ColorRGB PathIntegrator::Li(const Ray& r, const Scene& scene, Sampler& sampler)
{
	auto ray = r;
	ColorRGB Li{0, 0, 0};
	Vector3f beta{1, 1, 1};
	bool specularBounce = false;
	float bsdfPdf = 1.f;
	for (auto depth : std::views::iota(0, maxDepth)) {
		HitRecord primRec;
		if (!scene.hit(ray, primRec)) {
#if DEBUG_LAST_PATH_CONTRIB_ONLY
			for (const auto& infLight : scene.getInfiniteLights()) {
				if (depth == 0 || specularBounce) {
					Li = beta.cwiseProduct(infLight->getRadiance({}, ray.d));
				}
				else {
					const auto misWeight =
						powerHeuristic(1, infLight->getPdf({}, ray.d), 1, bsdfPdf);
					Li = beta.cwiseProduct(infLight->getRadiance({}, ray.d)) * misWeight;
				}
			}
#else
			for (const auto& infLight : scene.getInfiniteLights()) {
				if (depth == 0 || specularBounce) {
					Li += beta.cwiseProduct(infLight->getRadiance({}, ray.d));
				}
				else {
					const auto misWeight =
						powerHeuristic(1, bsdfPdf, 1, infLight->getPdf({}, ray.d));
					Li += beta.cwiseProduct(infLight->getRadiance({}, ray.d)) * misWeight;
				}
			}
#endif

			break;
		}

		const auto shadingGeom = primRec.getShadingGeomParams();
		const auto chooseLightRes = lightSampler->sampleLight(sampler.sample1D(), shadingGeom);
#if DEBUG_LAST_PATH_CONTRIB_ONLY
		Li = beta.cwiseProduct(SampleLightContribution(ray, primRec, sampler.sample2D(),
													   *chooseLightRes.light, scene) /
							   chooseLightRes.pmf);
#else
		Li += beta.cwiseProduct(SampleLightContribution(ray, primRec, sampler.sample2D(),
														*chooseLightRes.light, scene) /
								chooseLightRes.pmf);
#endif

		const auto bxdfDirPdf = primRec.sampleBxDFPdf(sampler.sample2D(), -ray.d);
		const Vector3f& newWi = bxdfDirPdf.dir;
		bsdfPdf = bxdfDirPdf.pdf;
		const ColorRGB& bxdfVal = bxdfDirPdf.bxdf;
		if (bxdfVal.isApprox(ColorRGB{0, 0, 0}))
			break;
		ray = primRec.spawnRay(newWi);
		beta =
			beta.cwiseProduct(bxdfVal * std::fabs(newWi.dot(shadingGeom.derivatives.n)) / bsdfPdf);
		specularBounce = primRec.getPhysicalPlausibleMaterial()->isDelta();
		// constexpr int minDepth = 3;
		// if (depth >= minDepth) {
		//	const auto q = std::max(0.05f, beta.maxCoeff());
		//	if (sampler.sample1D() < q)
		//		break;
		//	beta /= 1 - q;
		// }
	}

	return Li;
}

void PathIntegrator::preProcess(const Scene& scene)
{
	SamplerIntegrator::preProcess(scene);
	lightSampler = std::make_shared<SimpleLightSampler>(scene);
}

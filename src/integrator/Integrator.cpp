//
// Created by Frank on 2023/12/1.
//
#include "Integrator.h"
#include "HitRecord.h"
#include "Light.h"
#include "MathUtil.h"
#include "Primitive.h"
#include "Ray.h"
#include "Scene.h"
namespace xd {
ColorRGB EstimateDirect(const Ray& primRay,
						const HitRecord& primRec,
						const Vector2f& uLight,
						const Light& light,
						const Vector2f& uBxdf,
						const Scene& scene)
{
	ColorRGB Li{0, 0, 0};
	HitRecord shadowRec;
	float lightPdf, bsdfPdf;

	const auto& material = primRec.primitive->getMaterial();
	const auto calLi = [&](const Vector3f& shadowRayDir) -> Vector3f {
		const Ray shadowRay{primRec.p, shadowRayDir};
		if (!scene.hitAnything(shadowRay, shadowRec)) {
			const float cosTheta = std::clamp(primRec.n.dot(shadowRay.d), 0.f, 1.f);
			const ColorRGB projectedRadiance = light.getRadiance(primRec, shadowRayDir) * cosTheta;
			const Vector3f brdf = material->getBRDF(primRec, -primRay.d, shadowRay.d);
			return projectedRadiance.cwiseProduct(brdf);
		}
		else
			return {0, 0, 0};
	};
	const auto lightWi = light.sampleDirectionWithPdf(uLight, primRec, shadowRec, lightPdf);
	Vector3f lightLi = calLi(lightWi);
	if (!light.isDelta()) {
		bsdfPdf = material->getPdf(primRec, lightWi);
		const auto lightWeight = powerHeuristic(1, lightPdf, 1, bsdfPdf);
		Li += lightLi * lightWeight / lightPdf;
		// sample bsdf
		shadowRec = {};
		const auto bsdfWi = material->sampleDirectionWithPdf(uBxdf, -primRay.d, primRec, bsdfPdf);
		constexpr float eps = 1e-5f;
		if (bsdfWi.cwiseGreater(eps).any()) {
			Vector3f bsdfLi = calLi(bsdfWi);
			lightPdf = light.getPdf(primRec, bsdfWi);
			const auto bsdfWeight = powerHeuristic(1, bsdfPdf, 1, lightPdf);
			Li += bsdfLi * bsdfWeight / bsdfPdf;
		}
	}
	else
		Li += lightLi / lightPdf;
	return Li;
}
}  // namespace xd

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
#include "light/DomeLight.h"
namespace xd {
ColorRGB EstimateDirect(const Ray& primRay,
						const HitRecord& primRec,
						const Vector2f& uLight,
						const Light& light,
						const Vector2f& uBxdf,
						const Scene& scene)
{
	ColorRGB Li{0, 0, 0};

	const auto& material = primRec.primitive->getMaterial();
	// sample from light's direction
#if 1
	{
		Vector3f lightWi;
		float lightPdf;
		HitRecord shadowRec{};
		const auto lightRadiance =
			light.sampleRadianceWithPdf(uLight, primRec, shadowRec, lightWi, lightPdf);
		if (!isBlack(lightRadiance) && lightPdf > 0) {
			const auto shadowRay = primRec.spawnRay(lightWi);
			if (!scene.hitAnything(shadowRay, shadowRec)) {
				const float cosTheta = std::clamp(primRec.n.dot(shadowRay.d), 0.f, 1.f);
				const ColorRGB projectedRadiance = lightRadiance * cosTheta;
				const ColorRGB bsdf = material->getBxDF(primRec, -primRay.d, shadowRay.d);
				const auto bsdfPdf = material->getPdf(primRec, lightWi);
				if (!isBlack(bsdf) && bsdfPdf > 0) {
					if (light.isDelta()) {
						Li += projectedRadiance.cwiseProduct(bsdf) / lightPdf;
					}
					else {
						const auto weight = powerHeuristic(1, lightPdf, 1, bsdfPdf);
						Li += projectedRadiance.cwiseProduct(bsdf) * weight / lightPdf;
					}
				}
			}
		}
	}
#endif
	// sample from bsdf's direction
#if 1
	if (!light.isDelta()) {
		Vector3f bsdfWi;
		float bsdfPdf;
		const auto bsdf = material->sampleBxDFWithPdf(uBxdf, primRec, -primRay.d, bsdfWi, bsdfPdf);
		if (!isBlack(bsdf) && bsdfPdf > 0) {
			const auto shadowRay = primRec.spawnRay(bsdfWi);
			HitRecord shadowRec;
			if (scene.hitAnything(shadowRay, shadowRec)) {
				// TODO: handle area light
			}
			else {
				// dome light goes here
				const auto* dome = dynamic_cast<const DomeLight*>(&light);
				if (dome != nullptr) {
					const auto lightRadiance = dome->getRadiance(primRec, bsdfWi);
					const float cosTheta = std::clamp(primRec.n.dot(shadowRay.d), 0.f, 1.f);
					const ColorRGB projectedRadiance = lightRadiance * cosTheta;
					if (!material->isDelta()) {
						const auto lightPdf = dome->getPdf(primRec, bsdfWi);
						const auto weight = powerHeuristic(1, bsdfPdf, 1, lightPdf);
						Li += projectedRadiance.cwiseProduct(bsdf) * weight / bsdfPdf;
					}
					else {
						Li += projectedRadiance.cwiseProduct(bsdf) / bsdfPdf;
					}
					//
				}
			}
		}
	}
#endif
	return Li;
}
}  // namespace xd

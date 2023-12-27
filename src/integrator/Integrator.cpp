//
// Created by Frank on 2023/12/1.
//
#include "Integrator.h"
#include "HitRecord.h"
#include "Light.h"
#include "LightSampler.h"
#include "MathUtil.h"
#include "Primitive.h"
#include "Ray.h"
#include "Scene.h"
#include "light/DomeLight.h"
namespace xd {
ColorRGB SampleLightContribution(const Ray& primRay,
								 const HitRecord& primRec,
								 const Vector2f& uLight,
								 const Light& light,
								 const Scene& scene)
{
	ColorRGB Li = {0, 0, 0};

	const auto shadingGeom = primRec.getShadingGeomParams();
	// sample from light's direction
	if (!primRec.getPhysicalPlausibleMaterial()->isDelta()) {
		const auto radianceDirPdf = light.sampleRadianceWithPdf(uLight, shadingGeom);
		HitRecord shadowRec{};
		if (!light.isInfinite())
			shadowRec.tHit = radianceDirPdf.geomToLight.norm();
		const auto lightWi = radianceDirPdf.geomToLight.normalized();
		const auto lightPdf = radianceDirPdf.pdf;
		const auto& lightRadiance = radianceDirPdf.radiance;
		if (!isBlack(lightRadiance) && lightPdf > 0) {
			const auto shadowRay = primRec.spawnRay(lightWi);
			if (!scene.hitAnything(shadowRay, shadowRec)) {
				const float cosTheta =
					std::clamp(shadingGeom.derivatives.n.dot(shadowRay.d), 0.f, 1.f);
				const ColorRGB projectedRadiance = radianceDirPdf.radiance * cosTheta;
				const ColorRGB bsdf = primRec.getBxDF(-primRay.d, shadowRay.d);
				const float bsdfPdf = primRec.getMaterialPdf(lightWi);
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
	return Li;
}

ColorRGB SampleMaterialContribution(const Ray& primRay,
									const HitRecord& primRec,
									const Vector2f& uBxdf,
									float uChooseLight,
									const Scene& scene,
									const LightSampler* lightSampler,
									bool sampleSpecular)
{
	ColorRGB Li{0, 0, 0};
	const auto shadingGeom = primRec.getShadingGeomParams();
	const auto mtl = primRec.getPhysicalPlausibleMaterial();
	if (!mtl)
		return Li;
	const auto getLightRadianceFunc = [&](const std::shared_ptr<Light>& light) -> ColorRGB {
		if (!light->isDelta() && !(!sampleSpecular && mtl->isDelta())) {
			const auto bsdfDirPdf = primRec.sampleBxDFPdf(uBxdf, -primRay.d);
			const Vector3f& bsdfWi = bsdfDirPdf.dir;
			const ColorRGB& bsdf = bsdfDirPdf.bxdf;
			const float bsdfPdf = bsdfDirPdf.pdf;
			if (!isBlack(bsdf) && bsdfPdf > 0) {
				const auto shadowRay = primRec.spawnRay(bsdfWi);
				HitRecord shadowRec;
				if (scene.hitAnything(shadowRay, shadowRec)) {
					return {0, 0, 0};
				}
				else {
					const auto lightRadiance = light->getRadiance(shadingGeom, bsdfWi);
					const float cosTheta =
						std::clamp(shadingGeom.derivatives.n.dot(shadowRay.d), 0.f, 1.f);
					const ColorRGB projectedRadiance = lightRadiance * cosTheta;
					if (!mtl->isDelta()) {
						const auto lightPdf = light->getPdf(shadingGeom, bsdfWi);
						const auto weight = powerHeuristic(1, bsdfPdf, 1, lightPdf);
						return projectedRadiance.cwiseProduct(bsdf) * weight / bsdfPdf;
					}
					else {
						return projectedRadiance.cwiseProduct(bsdf) / bsdfPdf;
					}
				}
			}
		}
		return {0, 0, 0};
	};
	if (lightSampler) {
		const auto chooseLightRes = lightSampler->sampleLight(uChooseLight, shadingGeom);
		Li += getLightRadianceFunc(chooseLightRes.light) / chooseLightRes.pmf;
	}
	else {
		for (const auto& light : scene.getLights()) {
			Li += getLightRadianceFunc(light);
		}
	}

	return Li;
}

ColorRGB EstimateDirect(const Ray& primRay,
						const HitRecord& primRec,
						const Vector2f& uLight,
						const Light& light,
						const Vector2f& uBxdf,
						float uChooseLight,
						const Scene& scene,
						const LightSampler* lightSampler,
						bool sampleSpecular)
{
	ColorRGB Li{0, 0, 0};
	Li += SampleLightContribution(primRay, primRec, uLight, light, scene);
	// sample from bsdf's direction
	Li += SampleMaterialContribution(primRay, primRec, uBxdf, uChooseLight, scene, lightSampler,
									 sampleSpecular);
	return Li;
}
}  // namespace xd

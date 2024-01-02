//
// Created by Frank on 2023/12/16.
//
#include "PerfectTransmissionMaterial.h"
#include <cassert>
#include "HitRecord.h"
#include "MathUtil.h"
using namespace xd;
PerfectTransmissionMaterial::PerfectTransmissionMaterial(float etaOutside, float etaInside)
	: outToIn(std::make_unique<PerfectTransmission>(etaOutside, etaInside)),
	  inToOut(std::make_unique<PerfectTransmission>(etaInside, etaOutside))
{
}

PerfectTransmissionMaterial::PerfectTransmissionMaterial(
	std::shared_ptr<Texture2DRGB> normalTexture,
	float etaOutside,
	float etaInside)
	: PhysicalPlausibleMaterial(std::move(normalTexture)),
	  outToIn(std::make_unique<PerfectTransmission>(etaOutside, etaInside)),
	  inToOut(std::make_unique<PerfectTransmission>(etaInside, etaOutside))
{
}

ColorRGB PerfectTransmissionMaterial::getBxDF(const LocalGeomParams& shadingGeom,
											  const Vector3f& wo,
											  const Vector3f& wi) const
{
	return {0, 0, 0};
}

SampleBxDFResult PerfectTransmissionMaterial::sampleBxDF(const Vector2f& uSample,
														 const LocalGeomParams& shadingGeom,
														 const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(shadingGeom.derivatives.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(shadingGeom, flipNormal);
	auto ret = btdf->sampleBxDF(uSample, (worldToLocal * wo).normalized());
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}

SampleBxDFPdfResult PerfectTransmissionMaterial::sampleBxDFWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom,
	const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(shadingGeom.derivatives.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(shadingGeom, flipNormal);
	auto ret = btdf->sampleBxDFWithPdf(uSample, worldToLocal * wo);
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}
float PerfectTransmissionMaterial::getPdf(const LocalGeomParams& shadingGeom,
										  const Vector3f& wo) const
{
	return 0;
}
Vector3f PerfectTransmissionMaterial::sampleDirection(const Vector2f& uSample,
													  const LocalGeomParams& shadingGeom,
													  const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(shadingGeom.derivatives.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(shadingGeom, flipNormal);
	return applyTransformToDirection(localToWorld,
									 btdf->sampleDirection(uSample, worldToLocal * wo));
}

SampleDirPdfResult PerfectTransmissionMaterial::sampleDirectionWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom,
	const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(shadingGeom.derivatives.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(shadingGeom, flipNormal);
	auto ret = btdf->sampleDirectionWithPdf(uSample, worldToLocal * wo);
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}
bool PerfectTransmissionMaterial::isDelta() const
{
	return true;
}

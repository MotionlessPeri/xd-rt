//
// Created by Frank on 2023/9/18.
//
#include "PerfectReflectionMaterial.h"
#include <cassert>
#include "HitRecord.h"
#include "bxdf/PerfectReflection.h"
using namespace xd;
PerfectReflectionMaterial::PerfectReflectionMaterial() : brdf(std::make_unique<PerfectReflection>())
{
}

PerfectReflectionMaterial::PerfectReflectionMaterial(std::shared_ptr<Texture> normal)
	: PhysicalPlausibleMaterial(std::move(normal)), brdf(std::make_unique<PerfectReflection>())
{
}

ColorRGB PerfectReflectionMaterial::getBxDF(const LocalGeomParams& shadingGeom,
											const Vector3f& wo,
											const Vector3f& wi) const
{
	return {0, 0, 0};
}

SampleBxDFResult PerfectReflectionMaterial::sampleBxDF(const Vector2f& uSample,
													   const LocalGeomParams& shadingGeom,
													   const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto localToWorld = shadingGeom.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleBxDF(uSample, applyTransformToDirection(worldToLocal, wo));
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}

SampleBxDFPdfResult PerfectReflectionMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
																 const LocalGeomParams& shadingGeom,
																 const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto localToWorld = shadingGeom.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleBxDFWithPdf(uSample, worldToLocal * wo);
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}

Vector3f PerfectReflectionMaterial::sampleDirection(const Vector2f& uSample,
													const LocalGeomParams& shadingGeom,
													const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto localToWorld = shadingGeom.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	return (localToWorld * brdf->sampleDirection(uSample, worldToLocal * wo)).normalized();
}

SampleDirPdfResult PerfectReflectionMaterial::sampleDirectionWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom,
	const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto localToWorld = shadingGeom.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleDirectionWithPdf(uSample, worldToLocal * wo);
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}

float PerfectReflectionMaterial::getPdf(const LocalGeomParams& shadingGeom,
										const Vector3f& wo) const
{
	return 0;
}

//
// Created by Frank on 2023/9/2.
//
#include "MatteMaterial.h"
#include <cassert>
#include "BxDF.h"
#include "HitRecord.h"
#include "bxdf/Lambertian.h"
#include "texture/ConstantTexture.h"
using namespace xd;
MatteMaterial::MatteMaterial(const ColorRGB& c)
{
	color = std::make_shared<ConstantTexture>(c);
}
MatteMaterial::MatteMaterial(std::shared_ptr<Texture> colorTexture)
	: color(std::move(colorTexture))
{
}

MatteMaterial::MatteMaterial(std::shared_ptr<Texture> normalTexture, const ColorRGB& c)
	: PhysicalPlausibleMaterial(std::move(normalTexture))
{
	color = std::make_shared<ConstantTexture>(c);
}

MatteMaterial::MatteMaterial(std::shared_ptr<Texture> normalTexture,
							 std::shared_ptr<Texture> colorTexture)
	: PhysicalPlausibleMaterial(std::move(normalTexture)), color(std::move(colorTexture))
{
}

ColorRGB MatteMaterial::getBxDF(const LocalGeomParams& shadingGeom,
								const Vector3f& wo,
								const Vector3f& wi) const
{
	const auto sampledColor = color->sample(TextureEvalContext(shadingGeom)).head<3>();
	const Lambertian lambertian(sampledColor);
	Vector3f dummy;
	return lambertian.getBxDF(dummy, dummy);
}

SampleBxDFResult MatteMaterial::sampleBxDF(const Vector2f& uSample,
										   const LocalGeomParams& shadingGeom,
										   const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto localToWorld = shadingGeom.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto sampledColor = color->sample(TextureEvalContext(shadingGeom)).head<3>();
	const Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleBxDF(uSample, worldToLocal * wo);
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}

SampleBxDFPdfResult MatteMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
													 const LocalGeomParams& shadingGeom,
													 const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto localToWorld = shadingGeom.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto sampledColor = color->sample(TextureEvalContext(shadingGeom)).head<3>();
	const Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleBxDFWithPdf(uSample, worldToLocal * wo);
	ret.dir = applyTransformToDirection(localToWorld, ret.dir);
	return ret;
}

Vector3f MatteMaterial::sampleDirection(const Vector2f& uSample,
										const LocalGeomParams& shadingGeom,
										const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	auto sampledColor = color->sample(TextureEvalContext(shadingGeom)).head<3>();
	const Lambertian lambertian(sampledColor);
	return applyTransformToDirection(shadingGeom.getCurrentFrame(),
									 lambertian.sampleDirection(uSample, {0, 0, 1}));  // dummy wi
}

SampleDirPdfResult MatteMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
														 const LocalGeomParams& shadingGeom,
														 const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto sampledColor = color->sample(TextureEvalContext(shadingGeom)).head<3>();
	const Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleDirectionWithPdf(uSample, {0, 0, 1});  // dummy wi
	ret.dir = applyTransformToDirection(shadingGeom.getCurrentFrame(), ret.dir);
	return ret;	 // dummy wi
}

float MatteMaterial::getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const
{
	assert(shadingGeom.frame == FrameCategory::WORLD);
	const auto sampledColor = color->sample(TextureEvalContext(shadingGeom)).head<3>();
	const Lambertian lambertian(sampledColor);
	return lambertian.getPdf(shadingGeom.getCurrentFrame() * wo);
}

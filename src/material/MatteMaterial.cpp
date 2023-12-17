//
// Created by Frank on 2023/9/2.
//
#include "MatteMaterial.h"
#include "BxDF.h"
#include "HitRecord.h"
#include "bxdf/Lambertian.h"
#include "texture/ConstantTexture.h"
using namespace xd;
MatteMaterial::MatteMaterial(const ColorRGB& c)
{
	color = std::make_shared<ConstantTexture<ColorRGB, Vector2f>>(c);
}
MatteMaterial::MatteMaterial(const std::shared_ptr<Texture2DRGB>& colorTexture)
	: color(colorTexture)
{
}

ColorRGB MatteMaterial::getBxDF(const HitRecord& primRec,
								const Vector3f& wo,
								const Vector3f& wi) const
{
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	Vector3f dummy;
	return lambertian.getBxDF(dummy, dummy);
}

ColorRGB MatteMaterial::sampleBxDF(const Vector2f& uSample,
								   const HitRecord& primRec,
								   const Vector3f& wo,
								   Vector3f& wi)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleBxDF(uSample, worldToLocal * wo, wi);
	wi = localToWorld * wi;
	return ret;
}

ColorRGB MatteMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
										  const HitRecord& primRec,
										  const Vector3f& wo,
										  Vector3f& wi,
										  float& pdf)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleBxDFWithPdf(uSample, worldToLocal * wo, wi, pdf);
	wi = localToWorld * wi;
	return ret;
}

Vector3f MatteMaterial::sampleDirection(const Vector2f& uSample,
										const HitRecord& primRec,
										const Vector3f& wo) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	return primRec.getCurrentFrame() * lambertian.sampleDirection(uSample, {0, 0, 1});	// dummy wi
}
Vector3f MatteMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
											   const HitRecord& primRec,
											   const Vector3f& wo,
											   float& pdf) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	return primRec.getCurrentFrame() *
		   lambertian.sampleDirectionWithPdf(uSample, {0, 0, 1}, pdf);	// dummy wi
}
float MatteMaterial::getPdf(const HitRecord& primRec, const Vector3f& wo) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	return lambertian.getPdf(primRec.getCurrentFrame() * wo);
}

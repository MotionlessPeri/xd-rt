//
// Created by Frank on 2023/9/2.
//
#include "BxDF.h"
#include "HitRecord.h"
#include "Material.h"
#include "Texture.h"
using namespace xd;
MatteMaterial::MatteMaterial(const Vector3f& c)
{
	color = std::make_shared<ConstantTexture<Vector3f, Vector2f>>(c);
}
MatteMaterial::MatteMaterial(const std::shared_ptr<Texture2DRGB>& colorTexture)
	: color(colorTexture)
{
}

ColorRGB MatteMaterial::getBRDF(const HitRecord& primRec,
								const Vector3f& wo,
								const Vector3f& wi) const
{
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	Vector3f dummy;
	return lambertian.getBRDF(dummy, dummy);
}

ColorRGB MatteMaterial::sampleBRDF(const Vector2f& uSample,
								   const HitRecord& primRec,
								   const Vector3f& wo,
								   Vector3f& wi)
{
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleBRDF(uSample, primRec.modelToLocal * wo, wi);
	wi = primRec.localToModel * wi;
	return ret;
}

ColorRGB MatteMaterial::sampleBRDFWithPdf(const Vector2f& uSample,
										  const HitRecord& primRec,
										  const Vector3f& wo,
										  Vector3f& wi,
										  float& pdf)
{
	auto sampledColor = color->sample(primRec.uv);
	Lambertian lambertian(sampledColor);
	auto ret = lambertian.sampleBRDFWithPdf(uSample, primRec.modelToLocal * wo, wi, pdf);
	wi = primRec.localToModel * wi;
	return ret;
}

Vector3f MatteMaterial::sampleDirection(const Vector2f& uSample,
										const Vector3f& wo,
										const HitRecord& hitRecord) const
{
	auto sampledColor = color->sample(hitRecord.uv);
	Lambertian lambertian(sampledColor);
	return hitRecord.localToModel * lambertian.sampleDirection(uSample, {0, 0, 1});	 // dummy wi
}
Vector3f MatteMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
											   const Vector3f& wo,
											   const HitRecord& hitRecord,
											   float& pdf) const
{
	auto sampledColor = color->sample(hitRecord.uv);
	Lambertian lambertian(sampledColor);
	return hitRecord.localToModel *
		   lambertian.sampleDirectionWithPdf(uSample, {0, 0, 1}, pdf);	// dummy wi
}
float MatteMaterial::getPdf(const HitRecord& hitRecord, const Vector3f& wo) const
{
	auto sampledColor = color->sample(hitRecord.uv);
	Lambertian lambertian(sampledColor);
	return lambertian.getPdf(hitRecord.modelToLocal * wo);
}

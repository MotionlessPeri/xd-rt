//
// Created by Frank on 2023/8/31.
//
#include "Lambertian.h"
#include "HitRecord.h"

using namespace xd;
Lambertian::Lambertian(ColorRGB color)
	: color(std::move(color)), distrib(std::make_unique<CosineHemisphere>())
{
}
ColorRGB Lambertian::getBxDF(const Vector3f& wi, const Vector3f& wo) const
{
	return color * INV_PI;
}

ColorRGB Lambertian::sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi)
{
	wi = sampleDirection(uSample, wo);
	return getBxDF(wi, wo);
}

ColorRGB Lambertian::sampleBxDFWithPdf(const Vector2f& uSample,
									   const Vector3f& wo,
									   Vector3f& wi,
									   float& pdf)
{
	wi = sampleDirectionWithPdf(uSample, wo, pdf);
	return getBxDF(wi, wo);
}

Vector3f Lambertian::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	return distrib->sample(uSample);
}

Vector3f Lambertian::sampleDirectionWithPdf(const Vector2f& uSample, const Vector3f& wo, float& pdf)
{
	const auto wi = sampleDirection(uSample, wo);
	pdf = getPdf(wi);
	return wi;
}

bool Lambertian::isDelta() const
{
	return false;
}
float Lambertian::getPdf(const Vector3f& wo) const
{
	return distrib->getPdf(wo);
}

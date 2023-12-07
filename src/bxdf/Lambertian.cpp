//
// Created by Frank on 2023/8/31.
//
#include "BxDF.h"
#include "HitRecord.h"
using namespace xd;
Lambertian::Lambertian(const ColorRGB& color)
	: color(color), distrib(std::make_unique<CosineHemisphere>())
{
}
ColorRGB Lambertian::getBRDF(const Vector3f& wi, const Vector3f& wo) const
{
	return color * INV_PI;
}

ColorRGB Lambertian::sampleBRDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi)
{
	wi = sampleDirection(uSample, wo);
	return getBRDF(wi, wo);
}

ColorRGB Lambertian::sampleBRDFWithPdf(const Vector2f& uSample,
									   const Vector3f& wo,
									   Vector3f& wi,
									   float& pdf)
{
	wi = sampleDirectionWithPdf(uSample, wo, pdf);
	return getBRDF(wi, wo);
}

Vector3f Lambertian::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	return distrib->sample(uSample);
}
bool Lambertian::isDelta() const
{
	return false;
}
float Lambertian::getPdf(const Vector3f& wo) const
{
	return distrib->getPdf(wo);
}
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

SampleBxDFResult Lambertian::sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto wi = sampleDirection(uSample, wo);
	return {wi, getBxDF(wi, wo)};
}

SampleBxDFPdfResult Lambertian::sampleBxDFWithPdf(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto wiPdf = sampleDirectionWithPdf(uSample, wo);
	return {getBxDF(wiPdf.dir, wo), wiPdf.dir, wiPdf.pdf};
}

Vector3f Lambertian::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	return distrib->sample(uSample);
}

SampleDirPdfResult Lambertian::sampleDirectionWithPdf(const Vector2f& uSample,
													  const Vector3f& wo) const
{
	const auto wi = sampleDirection(uSample, wo);
	return {wi, getPdf(wi)};
}

bool Lambertian::isDelta() const
{
	return false;
}
float Lambertian::getPdf(const Vector3f& wo) const
{
	return distrib->getPdf(wo);
}

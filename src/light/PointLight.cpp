//
// Created by Frank on 2023/8/29.
//
#include "PointLight.h"
#include "HitRecord.h"
using namespace xd;
PointLight::PointLight(Vector3f position, ColorRGB intensity)
	: Light(1u), position(std::move(position)), intensity(std::move(intensity))
{
}

Vector3f PointLight::sampleDirection(const Vector2f& uSample,
									 const LocalGeomParams& shadingGeom) const
{
	return position - shadingGeom.p;
}

Light::SampleDirectionPdfResult PointLight::sampleDirectionWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom) const
{
	return {sampleDirection(uSample, shadingGeom), 1.f};
}

bool PointLight::isInfinite() const
{
	return false;
}

float PointLight::getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const
{
	return 0;
}

ColorRGB PointLight::getRadiance(const LocalGeomParams& shadingGeom, const Vector3f& wi) const
{
	return intensity / (position - shadingGeom.p).squaredNorm();
}

Light::SampleRadianceResult PointLight::sampleRadiance(const Vector2f& uSample,
													   const LocalGeomParams& shadingGeom) const
{
	const auto& sampleDirRes = sampleDirection(uSample, shadingGeom);
	return {sampleDirRes, getRadiance(shadingGeom, sampleDirRes)};
}

Light::SampleRadiancePdfResult PointLight::sampleRadianceWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom) const
{
	const auto& sampleDirPdfRes = sampleDirectionWithPdf(uSample, shadingGeom);
	return {sampleDirPdfRes, getRadiance(shadingGeom, sampleDirPdfRes.geomToLight)};
}

//
// Created by Frank on 2023/8/29.
//
#include "HitRecord.h"
#include "Light.h"
#include "Ray.h"
using namespace xd;
PointLight::PointLight(const Vector3f& position, const ColorRGB& intensity)
	: Light(1u), position(position), intensity(intensity)
{
}

Vector3f PointLight::sampleDirection(const Vector2f& uSample,
									 const HitRecord& primRec,
									 HitRecord& shadowRec) const
{
	const Vector3f pp = (position - primRec.p);
	shadowRec.tHit = pp.norm();
	return pp.normalized();
}
Vector3f PointLight::sampleDirectionWithPdf(const Vector2f& uSample,
											const HitRecord& primRec,
											HitRecord& shadowRec,
											float& pdf) const
{
	const auto wo = sampleDirection(uSample, primRec, shadowRec);
	pdf = 1.f;
	return wo;
}
float PointLight::getPdf(const HitRecord& primRec, const Vector3f& wo) const
{
	return 0;
}
ColorRGB PointLight::getRadiance(const HitRecord& primRec, const Vector3f& wi) const
{
	return intensity / (position - primRec.p).squaredNorm();
}

ColorRGB PointLight::sampleRadiance(const Vector2f& uSample,
									const HitRecord& primRec,
									HitRecord& shadowRec,
									Vector3f& wi)
{
	wi = sampleDirection(uSample, primRec, shadowRec);
	return getRadiance(primRec, wi);
}

ColorRGB PointLight::sampleRadianceWithPdf(const Vector2f& uSample,
										   const HitRecord& primRec,
										   HitRecord& shadowRec,
										   Vector3f& wi,
										   float& pdf)
{
	wi = sampleDirectionWithPdf(uSample, primRec, shadowRec, pdf);
	return getRadiance(primRec, wi);
}

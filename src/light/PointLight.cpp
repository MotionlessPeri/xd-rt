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
ColorRGB PointLight::getIntensity(const Ray& ray) const
{
	return intensity / (position - ray.o).squaredNorm();
}
Vector3f PointLight::getDirection(const HitRecord& primRec, HitRecord& shadowRec) const
{
	const Vector3f pp = (position - primRec.tPoint);
	shadowRec.tHit = pp.norm();
	return pp.normalized();
}
float PointLight::getPdf(const Vector3f& dir) const
{
	return 0;
}
Vector3f PointLight::sample(const HitRecord& primRec, HitRecord& shadowRec, float& pdf)
{
	const auto wo = getDirection(primRec, shadowRec);
	pdf = getPdf(wo);
	return wo;
}

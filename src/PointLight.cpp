//
// Created by Frank on 2023/8/29.
//
#include "HitRecord.h"
#include "Light.h"
#include "Ray.h"
using namespace xd;
PointLight::PointLight(const Vector3f& position, const ColorRGB& intensity)
	: position(position), intensity(intensity)
{
}
ColorRGB PointLight::getIntensity(const Vector3f& point) const
{
	return intensity;
}
Ray PointLight::getShadowRay(const Vector3f& point, HitRecord& record) const
{
	const Vector3f pp = position - point;
	record.tHit = pp.norm();
	return {point, pp.normalized()};
}

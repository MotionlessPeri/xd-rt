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
ColorRGB PointLight::getIntensity(const Vector3f& direction) const
{
	return intensity;
}
Vector3f PointLight::getDirection(const Vector3f& point, HitRecord& rec) const
{
	const Vector3f pp = (position - point);
	rec.tHit = pp.norm();
	return pp.normalized();
}

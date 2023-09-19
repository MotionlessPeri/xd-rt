//
// Created by Frank on 2023/9/19.
//
#include "Light.h"
#include "Ray.h"
#include "Texture.h"
using namespace xd;
DomeLight::DomeLight(const std::shared_ptr<SphereTexture3f>& dome) : dome(dome) {}
Vector3f DomeLight::getDirection(const Vector3f& point, HitRecord& rec) const
{
	// TODO: return the direction according to dome's light distribution
	return dis();
}
ColorRGB DomeLight::getIntensity(const Vector3f& direction) const
{
	return dome->sample(direction);
}
bool DomeLight::isDeltaPosition() const
{
	return false;
}
bool DomeLight::isDeltaDirection() const
{
	return false;
}

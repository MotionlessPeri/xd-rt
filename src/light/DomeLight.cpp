//
// Created by Frank on 2023/9/19.
//
#include "HitRecord.h"
#include "Light.h"
#include "Ray.h"
#include "Texture.h"
using namespace xd;
DomeLight::DomeLight(const std::shared_ptr<SphereTextureRGB>& dome) : Light(4u), dome(dome)
{
	const auto width = dome->getWidth();
	const auto height = dome->getHeight();
	const auto& data = dome->getImage();
	const auto pixelCnt = width * height;
	std::vector<float> weights(pixelCnt, 0.f);
	for (auto i = 0u; i < pixelCnt; ++i) {
		weights[i] = rgbToLuminance(data[i]);
	}
	dis = std::make_shared<PieceWise2D>(weights, width, height);
}
Vector3f DomeLight::getDirection(const HitRecord& primRec, HitRecord& shadowRec) const
{
	const auto uv = (*dis)();
	return getSphereDirFromUV(uv);
}
ColorRGB DomeLight::getIntensity(const Ray& ray) const
{
	return dome->sample(ray.d);
}
bool DomeLight::isDelta() const
{
	return false;
}
float DomeLight::getPdf(const Vector3f& dir) const
{
	const Vector2f uv = getSphereUV(dir);
	return dis->getPdf(uv);
}
Vector3f DomeLight::sample(const HitRecord& primRec, HitRecord& shadowRec, float& pdf)
{
	const auto uv = (*dis)(pdf);
	const auto wo = getSphereDirFromUV(uv);
	return wo;
}

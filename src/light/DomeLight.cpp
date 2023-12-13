//
// Created by Frank on 2023/9/19.
//
#include "HitRecord.h"
#include "Light.h"
#include "Ray.h"
#include "Texture.h"
using namespace xd;
DomeLight::DomeLight(const std::shared_ptr<SphereTextureRGB>& dome) : Light(1u), dome(dome)
{
	const auto width = dome->getWidth();
	const auto height = dome->getHeight();
	const auto& data = dome->getImage();
	const auto pixelCnt = width * height;
	std::vector<float> weights(pixelCnt, 0.f);
	for (auto i = 0u; i < pixelCnt; ++i) {
		weights[i] = rgbToLuminance(data[i]);
	}
	dis = std::make_unique<PieceWise2D>(weights, width, height);
}

Vector3f DomeLight::sampleDirection(const Vector2f& uSample,
									const HitRecord& primRec,
									HitRecord& shadowRec) const
{
	const auto uv = dis->sample(uSample);
	return getSphereDirFromUV(uv);
}

ColorRGB DomeLight::getRadiance(const HitRecord& primRec, const Vector3f& wi) const
{
	return dome->sample(wi);
}

ColorRGB DomeLight::sampleRadiance(const Vector2f& uSample,
								   const HitRecord& primRec,
								   HitRecord& shadowRec,
								   Vector3f& wi) const
{
	wi = sampleDirection(uSample, primRec, shadowRec);
	return getRadiance(primRec, wi);
}

ColorRGB DomeLight::sampleRadianceWithPdf(const Vector2f& uSample,
										  const HitRecord& primRec,
										  HitRecord& shadowRec,
										  Vector3f& wi,
										  float& pdf) const
{
	wi = sampleDirectionWithPdf(uSample, primRec, shadowRec, pdf);
	return getRadiance(primRec, wi);
}

bool DomeLight::isDelta() const
{
	return false;
}
float DomeLight::getPdf(const HitRecord& primRec, const Vector3f& wo) const
{
	const Vector2f uv = getSphereUV(wo);
	return dis->getPdf(uv);
}
Vector3f DomeLight::sampleDirectionWithPdf(const Vector2f& uSample,
										   const HitRecord& primRec,
										   HitRecord& shadowRec,
										   float& pdf) const
{
	const auto uv = dis->sampleWithPdf(uSample, pdf);
	const auto theta = uv[1] * PI;
	const auto phi = uv[0] * TWO_PI;
	const auto sinTheta = std::sinf(theta), cosTheta = std::cosf(theta);
	const auto sinPhi = std::sinf(phi), cosPhi = std::cosf(phi);
	pdf = pdf / (2 * PI * PI * sinTheta);
	auto wo = getSphereDirFromUV(uv);
	return wo;
}

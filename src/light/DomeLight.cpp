//
// Created by Frank on 2023/9/19.
//
#include "HitRecord.h"
#include "Light.h"
#include "Texture.h"
#include "texture/TextureFactory.h"
using namespace xd;
DomeLight::DomeLight(const ColorRGB& color) : Light(1u)
{
	dome = std::make_shared<ConstantTexture<ColorRGB, Vector3f>>(color);
	dis = std::make_unique<PieceWise2D>(std::vector<float>{1}, 1, 1);
}

DomeLight::DomeLight(const std::string& imagePath) : Light(1u)
{
	auto sphereTexture = TextureFactory::loadSphereTextureRGB(imagePath);
	const auto width = sphereTexture->getWidth();
	const auto height = sphereTexture->getHeight();
	const auto& data = sphereTexture->getImage();
	const auto pixelCnt = width * height;
	std::vector<float> weights(pixelCnt, 0.f);
	for (auto i = 0u; i < pixelCnt; ++i) {
		weights[i] = rgbToLuminance(data[i]);
	}
	dis = std::make_unique<PieceWise2D>(weights, width, height);
	dome = sphereTexture;
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
	const auto pdf = dis->getPdf(uv);
	return normalizePdf(pdf, uv);
}

Vector3f DomeLight::sampleDirectionWithPdf(const Vector2f& uSample,
										   const HitRecord& primRec,
										   HitRecord& shadowRec,
										   float& pdf) const
{
	const auto uv = dis->sampleWithPdf(uSample, pdf);
	pdf = normalizePdf(pdf, uv);
	auto wo = getSphereDirFromUV(uv);
	return wo;
}

float DomeLight::normalizePdf(float pieceWisePdf, const Vector2f& uv) const
{
	const auto sinTheta = std::sinf(uv[1] * PI);
	return pieceWisePdf / (2 * PI * PI * sinTheta);
}

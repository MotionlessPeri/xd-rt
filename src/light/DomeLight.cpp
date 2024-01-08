//
// Created by Frank on 2023/9/19.
//
#include "DomeLight.h"
#include "HitRecord.h"
#include "loader/TextureFactory.h"
#include "texture/ConstantTexture.h"
using namespace xd;
DomeLight::DomeLight(const ColorRGB& color) : Light(1u)
{
	dome = std::make_shared<ConstantTexture>(color);
	dis = std::make_unique<PieceWise2D>(std::vector<float>{1}, 1, 1);
}

DomeLight::DomeLight(const std::string& imagePath) : Light(1u)
{
	LoadTextureOptions options;
	options.filterType = FilterType::TENT;
	const auto sphereTexture = TextureFactory::get().loadSphereTexture(imagePath, options);
	const auto sphereImage = sphereTexture->getImage();
	const auto width = sphereImage->getWidth();
	const auto height = sphereImage->getHeight();
	const auto pixelCnt = width * height;
	std::vector<float> weights(pixelCnt, 0.f);
	for (auto i = 0u; i < pixelCnt; ++i) {
		weights[i] = rgbToLuminance(sphereImage->getPixelValue(i).head<3>());
	}
	dis = std::make_unique<PieceWise2D>(weights, width, height);
	dome = sphereTexture;
}

Vector3f DomeLight::sampleDirection(const Vector2f& uSample,
									const LocalGeomParams& shadingGeom) const
{
	const auto uv = dis->sample(uSample);
	return {getSphereDirFromUV(uv)};
}

ColorRGB DomeLight::getRadiance(const LocalGeomParams& shadingGeom, const Vector3f& wi) const
{
	return dome->sample(TextureEvalContext{wi}).head<3>();
}

Light::SampleRadianceResult DomeLight::sampleRadiance(const Vector2f& uSample,
													  const LocalGeomParams& shadingGeom) const
{
	auto&& sampleDirRes = sampleDirection(uSample, shadingGeom);
	return {sampleDirRes, getRadiance(shadingGeom, sampleDirRes)};
}

Light::SampleRadiancePdfResult DomeLight::sampleRadianceWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom) const
{
	auto&& sampleDirPdfRes = sampleDirectionWithPdf(uSample, shadingGeom);
	return {sampleDirPdfRes, getRadiance(shadingGeom, sampleDirPdfRes.geomToLight)};
}

bool DomeLight::isDelta() const
{
	return false;
}

float DomeLight::getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const
{
	const Vector2f uv = getSphereUV(wo);
	const auto pdf = dis->getPdf(uv);
	return normalizePdf(pdf, uv);
}

Light::SampleDirectionPdfResult DomeLight::sampleDirectionWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom) const
{
	float pdf;
	const auto uv = dis->sampleWithPdf(uSample, pdf);
	pdf = normalizePdf(pdf, uv);
	return {getSphereDirFromUV(uv), pdf};
}

bool DomeLight::isInfinite() const
{
	return true;
}

float DomeLight::normalizePdf(float pieceWisePdf, const Vector2f& uv)
{
	const auto sinTheta = std::sinf(uv[1] * PI);
	return pieceWisePdf / (2 * PI * PI * sinTheta);
}

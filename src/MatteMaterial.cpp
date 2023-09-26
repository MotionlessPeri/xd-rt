//
// Created by Frank on 2023/9/2.
//
#include "BRDF.h"
#include "HitRecord.h"
#include "Material.h"
#include "Texture.h"
using namespace xd;
MatteMaterial::MatteMaterial(const Vector3f& c)
{
	color = std::make_shared<ConstantTexture<Vector3f, Vector2f>>(c);
}
MatteMaterial::MatteMaterial(const std::shared_ptr<Texture2DC>& colorTexture) : color(colorTexture)
{
}
Vector3f MatteMaterial::getBRDF(const HitRecord& hitRecord,
								const Vector3f& wi,
								const Vector3f& wo) const
{
	auto sampledColor = color->sample(hitRecord.uv);
	Lambertian lambertian(sampledColor);
	Vector3f dummy;
	return lambertian.getBRDF(dummy, dummy);
}
Vector3f MatteMaterial::getDirection(const HitRecord& hitRecord, const Vector3f& wo) const
{
	auto sampledColor = color->sample(hitRecord.uv);
	Lambertian lambertian(sampledColor);
	return lambertian.getDirection(hitRecord.n);
}
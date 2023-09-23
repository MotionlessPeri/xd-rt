//
// Created by Frank on 2023/8/31.
//
#include "BRDF.h"
#include "MathUtil.h"
using namespace xd;
Lambertian::Lambertian(const ColorRGB& color) : color(color) {}
ColorRGB Lambertian::getBRDF(const Vector3f& wi, const Vector3f& wo) const
{
	return color * INV_PI;
}
Vector3f Lambertian::getDirection(const Vector3f& wi) const
{
	const auto sample = distrib();
	const auto [tx, ty] = coordSystem(wi);
	Matrix3f localToWorld;
	localToWorld << tx, ty, wi;
	return localToWorld * sample;
}
bool Lambertian::isDelta() const
{
	return false;
}
float Lambertian::getPdf(const Vector3f& wo) const
{
	return distrib.getPdf(wo);
}
Vector3f Lambertian::sample(const Vector3f& wi, float& pdf)
{
	const auto wo = getDirection(wi);
	pdf = getPdf(wo);
	return wo;
}

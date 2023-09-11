//
// Created by Frank on 2023/8/31.
//
#include "BRDF.h"
using namespace xd;
Lambertian::Lambertian(const ColorRGB& color) : color(color) {}
ColorRGB Lambertian::getBRDF(const Vector3f& wi, const Vector3f& wo) const
{
	return color * INV_PI;
}
Vector3f Lambertian::getDirection(const Vector3f& wi) const
{
	return distrib();
}
bool Lambertian::isDelta() const
{
	return false;
}

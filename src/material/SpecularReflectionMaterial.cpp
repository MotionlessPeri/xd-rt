//
// Created by Frank on 2023/9/18.
//
#include "HitRecord.h"
#include "Material.h"
#include "MathUtil.h"
using namespace xd;
Vector3f SpecularReflectionMaterial::getBRDF(const HitRecord& hitRecord,
											 const Vector3f& wi,
											 const Vector3f& wo) const
{
	const auto N = hitRecord.n;
	constexpr float eps = 1e-5;
	if (std::fabs(N.dot(wi) - N.dot(wo)) < eps) {
		return {1, 1, 1};
	}
	else
		return {0, 0, 0};
}
Vector3f SpecularReflectionMaterial::getDirection(const HitRecord& hitRecord,
												  const Vector3f& wo) const
{
	return reflected(wo, hitRecord.n);
}

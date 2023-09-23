//
// Created by Frank on 2023/8/27.
//

#ifndef XD_RT_BRDF_H
#define XD_RT_BRDF_H
#include "CoreTypes.h"
#include "Distribution.h"
#include "MathType.h"
namespace xd {
class BRDF : public std::enable_shared_from_this<BRDF> {
public:
	virtual ~BRDF() = default;
	virtual ColorRGB getBRDF(const Vector3f& wi, const Vector3f& wo) const = 0;
	virtual Vector3f getDirection(const Vector3f& wi) const = 0;
	virtual bool isDelta() const = 0;
	virtual float getPdf(const Vector3f& wo) const = 0;
	virtual Vector3f sample(const Vector3f& wi, float& pdf) = 0;
};

class Lambertian : public BRDF {
public:
	explicit Lambertian(const ColorRGB& color);
	ColorRGB getBRDF(const Vector3f& wi, const Vector3f& wo) const override;
	Vector3f getDirection(const Vector3f& wi) const override;
	bool isDelta() const override;
	float getPdf(const Vector3f& wo) const override;
	Vector3f sample(const Vector3f& wi, float& pdf) override;

protected:
	ColorRGB color;
	CosineHemisphere distrib;
};
}  // namespace xd
#endif	// XD_RT_BRDF_H

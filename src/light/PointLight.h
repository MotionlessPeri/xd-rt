//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_POINTLIGHT_H
#define XD_RT_POINTLIGHT_H
#include "Light.h"
namespace xd {
class PointLight : public Light {
public:
	PointLight(Vector3f position, ColorRGB intensity);
	Vector3f sampleDirection(const Vector2f& uSample,
							 const HitRecord& primRec,
							 HitRecord& shadowRec) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									HitRecord& shadowRec,
									float& pdf) const override;
	ColorRGB getRadiance(const HitRecord& primRec, const Vector3f& wi) const override;
	ColorRGB sampleRadiance(const Vector2f& uSample,
							const HitRecord& primRec,
							HitRecord& shadowRec,
							Vector3f& wi) const override;
	ColorRGB sampleRadianceWithPdf(const Vector2f& uSample,
								   const HitRecord& primRec,
								   HitRecord& shadowRec,
								   Vector3f& wi,
								   float& pdf) const override;
	bool isDelta() const override { return true; }
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;

protected:
	Vector3f position;
	ColorRGB intensity;
};
}  // namespace xd
#endif	// XD_RT_POINTLIGHT_H

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
							 const LocalGeomParams& shadingGeom) const override;
	SampleDirectionPdfResult sampleDirectionWithPdf(
		const Vector2f& uSample,
		const LocalGeomParams& shadingGeom) const override;
	ColorRGB getRadiance(const LocalGeomParams& shadingGeom, const Vector3f& wi) const override;
	SampleRadianceResult sampleRadiance(const Vector2f& uSample,
										const LocalGeomParams& shadingGeom) const override;
	SampleRadiancePdfResult sampleRadianceWithPdf(
		const Vector2f& uSample,
		const LocalGeomParams& shadingGeom) const override;
	bool isDelta() const override { return true; }
	bool isInfinite() const override;
	float getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const override;

protected:
	Vector3f position;
	ColorRGB intensity;
};
}  // namespace xd
#endif	// XD_RT_POINTLIGHT_H

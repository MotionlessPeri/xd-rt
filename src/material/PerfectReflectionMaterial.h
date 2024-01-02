//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PERFECTREFLECTIONMATERIAL_H
#define XD_RT_PERFECTREFLECTIONMATERIAL_H
#include "Material.h"
#include "bxdf/PerfectReflection.h"
namespace xd {

class PerfectReflectionMaterial : public PhysicalPlausibleMaterial {
public:
	PerfectReflectionMaterial();
	explicit PerfectReflectionMaterial(std::shared_ptr<Texture2DRGB> normal);

	ColorRGB getBxDF(const LocalGeomParams& shadingGeom,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	SampleBxDFResult sampleBxDF(const Vector2f& uSample,
								const LocalGeomParams& shadingGeom,
								const Vector3f& wo) const override;
	SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
										  const LocalGeomParams& shadingGeom,
										  const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const LocalGeomParams& shadingGeom,
							 const Vector3f& wo) const override;
	SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
											  const LocalGeomParams& shadingGeom,
											  const Vector3f& wo) const override;
	float getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const override;
	bool isDelta() const override { return true; }

protected:
	std::unique_ptr<PerfectReflection> brdf;
};
}  // namespace xd
#endif	// XD_RT_PERFECTREFLECTIONMATERIAL_H

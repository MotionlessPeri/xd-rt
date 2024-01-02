//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_MATTEMATERIAL_H
#define XD_RT_MATTEMATERIAL_H
#include "Material.h"
#include "texture/TextureTypes.h"
namespace xd {
class MatteMaterial : public PhysicalPlausibleMaterial {
public:
	explicit MatteMaterial(const ColorRGB& color);
	explicit MatteMaterial(std::shared_ptr<Texture2DRGB> colorTexture);
	MatteMaterial(std::shared_ptr<Texture2DRGB> normalTexture, const ColorRGB& color);
	MatteMaterial(std::shared_ptr<Texture2DRGB> normalTexture,
				  std::shared_ptr<Texture2DRGB> colorTexture);
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
	bool isDelta() const override { return false; }

protected:
	std::shared_ptr<Texture2DRGB> color;
};
}  // namespace xd
#endif	// XD_RT_MATTEMATERIAL_H

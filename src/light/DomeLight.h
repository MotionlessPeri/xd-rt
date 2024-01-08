//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_DOMELIGHT_H
#define XD_RT_DOMELIGHT_H
#include "Light.h"
#include "distribution/DistributionTypes.h"
#include "distribution/PieceWise2D.h"
#include "texture/TextureTypes.h"
namespace xd {
class DomeLight : public Light {
public:
	explicit DomeLight(const ColorRGB& color);
	explicit DomeLight(const std::string& imagePath);
	Vector3f sampleDirection(const Vector2f& uSample,
							 const LocalGeomParams& shadingGeom) const override;
	ColorRGB getRadiance(const LocalGeomParams& shadingGeom, const Vector3f& wi) const override;
	SampleRadianceResult sampleRadiance(const Vector2f& uSample,
										const LocalGeomParams& shadingGeom) const override;
	SampleRadiancePdfResult sampleRadianceWithPdf(
		const Vector2f& uSample,
		const LocalGeomParams& shadingGeom) const override;
	bool isDelta() const override;
	float getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const override;
	SampleDirectionPdfResult sampleDirectionWithPdf(
		const Vector2f& uSample,
		const LocalGeomParams& shadingGeom) const override;
	bool isInfinite() const override;
	std::shared_ptr<Texture> dome;
	std::unique_ptr<PieceWise2D> dis;

protected:
	static float normalizePdf(float pieceWisePdf, const Vector2f& uv);
};
}  // namespace xd
#endif	// XD_RT_DOMELIGHT_H

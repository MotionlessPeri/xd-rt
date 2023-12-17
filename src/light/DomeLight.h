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
							 const HitRecord& primRec,
							 HitRecord& shadowRec) const override;
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
	bool isDelta() const override;
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									HitRecord& shadowRec,
									float& pdf) const override;
	std::shared_ptr<Texture3D<ColorRGB>> dome;
	std::unique_ptr<PieceWise2D> dis;

protected:
	float normalizePdf(float pieceWisePdf, const Vector2f& uv) const;
};
}  // namespace xd
#endif	// XD_RT_DOMELIGHT_H

//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_LIGHT_H
#define XD_RT_LIGHT_H

#include <memory>
#include "CoreTypes.h"
#include "Distribution.h"
#include "MathType.h"
namespace xd {
class Light {
public:
	explicit Light(uint32_t numSamples) : numSamples(numSamples) {}
	virtual ~Light() = default;
	/**
	 * \brief sample a direction from primRec.p to the light
	 * \param uSample the sample used to sample light direction
	 * \param primRec the local presentation of the hit point.
	 * \param shadowRec hit record for shadow ray. shadowRec.tHit will be updated to perform any hit
	 * query.
	 * \return the direction from primRec.p to the light
	 */
	virtual Vector3f sampleDirection(const Vector2f& uSample,
									 const HitRecord& primRec,
									 HitRecord& shadowRec) const = 0;
	/**
	 * \brief sample a direction from primRec.p to the light, return pdf as well.
	 * \param uSample the sample used to sample light direction.
	 * \param primRec the local presentation of the hit point.
	 * \param shadowRec hit record for shadow ray. shadowRec.tHit will be updated to perform any hit
	 * query.
	 * \param pdf pdf of returned direction will be assigned to this param.
	 * \return the direction from primRec.p to the light.
	 */
	virtual Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
											const HitRecord& primRec,
											HitRecord& shadowRec,
											float& pdf) const = 0;
	/**
	 * \brief return the radiance of a point in a certain direction.
	 * \param primRec the local presentation of the hit point.
	 * \param wi the light outgoing direction. Points from primRec.p to the light.
	 * \return the radiance reach the given point in direction wi.
	 */
	virtual ColorRGB getRadiance(const HitRecord& primRec, const Vector3f& wi) const = 0;
	/**
	 * \brief sample a direction and return the radiance emitted in that direction.
	 * \param uSample the sample used to sample light
	 * direction.
	 * \param primRec the local presentation of the hit point.
	 * \param shadowRec hit record for shadow ray. shadowRec.tHit will be updated to perform any hit
	 * query.
	 * \param wi the light outgoing direction. Points from primRec.p to the light.
	 * \return the radiance reach the given point in direction wi.
	 */
	virtual ColorRGB sampleRadiance(const Vector2f& uSample,
									const HitRecord& primRec,
									HitRecord& shadowRec,
									Vector3f& wi) const = 0;
	/**
	 * \brief sample a direction and return the radiance emitted in that direction. Pdf is also
	 * returned.
	 * \param uSample the sample used to sample light direction.
	 * \param primRec the local presentation of the hit point.
	 * \param shadowRec hit record for shadow ray. shadowRec.tHit will be updated to perform any hit
	 * query.
	 * \param wi the light outgoing direction. Points from primRec.p to the light.
	 * \param pdf the pdf of wi will be assigned to this param.
	 * \return the radiance reach the given point in direction wi.
	 */
	virtual ColorRGB sampleRadianceWithPdf(const Vector2f& uSample,
										   const HitRecord& primRec,
										   HitRecord& shadowRec,
										   Vector3f& wi,
										   float& pdf) const = 0;
	virtual bool isDelta() const = 0;
	virtual float getPdf(const HitRecord& primRec, const Vector3f& wo) const = 0;

	uint32_t getNumSamples() const { return numSamples; }

protected:
	uint32_t numSamples;
};

class PointLight : public Light {
public:
	PointLight(const Vector3f& position, const ColorRGB& intensity);
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
#endif	// XD_RT_LIGHT_H

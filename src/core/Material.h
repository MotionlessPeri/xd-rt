//
// Created by Frank on 2023/8/26.
//

#ifndef XD_RT_MATERIAL_H
#define XD_RT_MATERIAL_H
#include <memory>
#include "BxDF.h"
#include "CoreTypes.h"
#include "MathType.h"
namespace xd {
/**
 * Material is an aggregate of brdfs
 */
class Material {
public:
	virtual ~Material() = default;
	/**
	 * get bxdf value
	 * @param primRec the geometric properties around hit point
	 * @param wo the incoming direction. Wi must lies in world frame.
	 * @param wi the outgoing direction. Wo must lies in world frame.
	 * @return the brdf value respected to wi and wo
	 */
	virtual ColorRGB getBRDF(const HitRecord& primRec,
							 const Vector3f& wo,
							 const Vector3f& wi) const = 0;
	/**
	 * \brief sample the brdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param primRec the local presentation of the hit point
	 * \param wo the incoming direction. Wo must lies in world frame.
	 * \param wi the outgoing direction will be assigned to this param. Wi lies in world frame.
	 * \return the brdf value
	 */
	virtual ColorRGB sampleBRDF(const Vector2f& uSample,
								const HitRecord& primRec,
								const Vector3f& wo,
								Vector3f& wi) = 0;
	/**
	 * \brief sample the brdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param primRec the local presentation of the hit point
	 * \param wo the incoming direction. Wo must lies in world frame.
	 * \param wi the outgoing direction will be assigned to this param. Wi lies in world frame.
	 * \param pdf the pdf of wi will be assigned to this param.
	 * \return the brdf value
	 */
	virtual ColorRGB sampleBRDFWithPdf(const Vector2f& uSample,
									   const HitRecord& primRec,
									   const Vector3f& wo,
									   Vector3f& wi,
									   float& pdf) = 0;
	/**
	 * \brief get pdf value of given wo
	 * \param primRec the local representation of hit point
	 * \param wo the outgoing direction. Wo must lies in world frame
	 * \return the pdf value associate to wo
	 */
	virtual float getPdf(const HitRecord& primRec, const Vector3f& wo) const = 0;
	/**
	 * \brief sample wo's direction according to wi and bxdf's distribution
	 * \param uSample the sampled point used for sampling direction
	 * \param wo outgoing direction
	 * \param primRec the local representation of hit point
	 * \return wo's direction. Note that wo is in world frame
	 */
	virtual Vector3f sampleDirection(const Vector2f& uSample,
									 const HitRecord& primRec,
									 const Vector3f& wo) const = 0;
	virtual Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
											const HitRecord& primRec,
											const Vector3f& wo,
											float& pdf) const = 0;
	virtual bool isDelta() const = 0;
};

class MatteMaterial : public Material {
public:
	MatteMaterial(const ColorRGB& color);
	MatteMaterial(const std::shared_ptr<Texture2DRGB>& colorTexture);
	ColorRGB getBRDF(const HitRecord& primRec,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	ColorRGB sampleBRDF(const Vector2f& uSample,
						const HitRecord& primRec,
						const Vector3f& wo,
						Vector3f& wi) override;
	ColorRGB sampleBRDFWithPdf(const Vector2f& uSample,
							   const HitRecord& primRec,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const HitRecord& primRec,
							 const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									const Vector3f& wo,
									float& pdf) const override;
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;
	bool isDelta() const override { return false; }

protected:
	std::shared_ptr<Texture2DRGB> color;
};

class PerfectReflectionMaterial : public Material {
public:
	PerfectReflectionMaterial();
	ColorRGB getBRDF(const HitRecord& primRec,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	ColorRGB sampleBRDF(const Vector2f& uSample,
						const HitRecord& primRec,
						const Vector3f& wo,
						Vector3f& wi) override;
	ColorRGB sampleBRDFWithPdf(const Vector2f& uSample,
							   const HitRecord& primRec,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const HitRecord& primRec,
							 const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									const Vector3f& wo,
									float& pdf) const override;
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;
	bool isDelta() const override { return true; }

protected:
	std::unique_ptr<PerfectReflection> brdf;
};
}  // namespace xd
#endif	// XD_RT_MATERIAL_H

//
// Created by Frank on 2023/8/27.
//

#ifndef XD_RT_BXDF_H
#define XD_RT_BXDF_H
#include "CoreTypes.h"
#include "MathTypes.h"
namespace xd {
class BxDF {
public:
	virtual ~BxDF() = default;
	virtual ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const = 0;
	/**
	 * \brief sample the brdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param wo the incoming direction. Wo must lies in local frame.
	 * \param wi the outgoing direction will be assigned to wi. Wi lies in the local frame.
	 * \return the brdf value
	 */
	virtual ColorRGB sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi) = 0;
	/**
	 * \brief sample the brdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param wo the incoming direction. Wo must lies in local frame.
	 * \param wi the outgoing direction will be assigned to wi. Wi lies in the local frame.
	 * \param pdf the pdf of wi will be assigned to this param.
	 * \return the brdf value
	 */
	virtual ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
									   const Vector3f& wo,
									   Vector3f& wi,
									   float& pdf) = 0;
	/**
	 * \brief sample wo's direction according to wi and bxdf's distribution
	 * \param uSample the sampled point used for sampling direction
	 * \param wo incoming direction
	 * \return wo's direction. Note that wo is in local frame
	 */
	virtual Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const = 0;

	/**
	 * \brief sample wo's direction according to wi and bxdf's distribution, and returning pdf of wo
	 * \param uSample the sampled point used for sampling direction
	 * \param wo incoming direction
	 * \param pdf probability density of wo
	 * \return wo's direction. Note that wo is in local frame
	 */
	virtual Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
											const Vector3f& wo,
											float& pdf) = 0;
	virtual float getPdf(const Vector3f& wi) const = 0;
	virtual bool isDelta() const = 0;
};

}  // namespace xd
#endif	// XD_RT_BXDF_H

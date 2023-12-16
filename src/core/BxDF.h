//
// Created by Frank on 2023/8/27.
//

#ifndef XD_RT_BXDF_H
#define XD_RT_BXDF_H
#include "CoreTypes.h"
#include "Distribution.h"
#include "MathType.h"
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

class Lambertian : public BxDF {
public:
	explicit Lambertian(const ColorRGB& color);
	ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi) override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const Vector3f& wo,
									float& pdf) override;
	float getPdf(const Vector3f& wo) const override;
	bool isDelta() const override;

protected:
	ColorRGB color;
	std::unique_ptr<CosineHemisphere> distrib;
};

class PerfectReflection : public BxDF {
public:
	~PerfectReflection() = default;
	ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi) override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const Vector3f& wo,
									float& pdf) override;
	float getPdf(const Vector3f& wo) const override;
	bool isDelta() const override;
};

/**
 * Perfect transmission respect of eta = eta_i / eta_t.
 * The implementation assumes the normal is in the same hemisphere of wi.
 * If the light travels into less dence medium, the ray will be neglected. Notice that because of
 * this, this bxdf is not energy preserved(it is energy conserved though)
 */
class PerfectTransmission : public BxDF {
public:
	explicit PerfectTransmission(float eta);
	PerfectTransmission(float etaI, float etaT);
	ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi) override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const Vector3f& wo,
									float& pdf) override;
	float getPdf(const Vector3f& wi) const override;
	bool isDelta() const override;

protected:
	Vector3f refract(const Vector3f& wo, float cosThetaT) const;
	/**
	 * \brief check if total reflection is reached. If reached, isTotal is set to true; otherwise
	 * sin2ThetaT is provided
	 * \param wo the incident direction
	 * \return {isTotal, sin2ThetaT}
	 */
	auto checkTotalReflection(const Vector3f& wo) const
	{
		struct Ret {
			bool isTotalReflection;
			float sin2ThetaT;
		} ret;
		const float cosThetaI = wo.z();
		const float sin2ThetaI = 1 - cosThetaI * cosThetaI;
		ret.sin2ThetaT = sin2ThetaI / (eta * eta);
		ret.isTotalReflection = ret.sin2ThetaT > 1;
		return ret;
	}
	float eta;
};
}  // namespace xd
#endif	// XD_RT_BXDF_H

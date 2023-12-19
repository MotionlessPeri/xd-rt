//
// Created by Frank on 2023/12/19.
//

#ifndef XD_RT_PERFECTFRESNELMATERIAL_H
#define XD_RT_PERFECTFRESNELMATERIAL_H
#include "Material.h"
#include "PerfectReflectionMaterial.h"
#include "PerfectTransmissionMaterial.h"
#include "bxdf/Fresnel.h"
namespace xd {

class PerfectFresnelMaterial : public Material {
public:
	PerfectFresnelMaterial(float etaOut, float etaIn);
	ColorRGB getBxDF(const HitRecord& primRec,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample,
						const HitRecord& primRec,
						const Vector3f& wo,
						Vector3f& wi) const override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const HitRecord& primRec,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) const override;
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const HitRecord& primRec,
							 const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									const Vector3f& wo,
									float& pdf) const override;
	bool isDelta() const override;

protected:
	/**
	 * The code template of determining sample reflection or transmission. Used by
	 * sample{BxDF, Direction}[WithPdf] methods
	 * @tparam SampleFuncType the actual sample function. The signature must be
	 * (bool sampleReflection, float fresnel) -> SomeType
	 * @param uSample the sampled point
	 * @param primRec the local presentation of the hit point
	 * @param wo the incoming direction. Wo must lies in world frame.
	 * @param func the function definition
	 * @return This template function returns what func returns
	 */
	template <typename SampleFuncType>
	auto sampleTemplate(const Vector2f& uSample,
						const HitRecord& primRec,
						const Vector3f& wo,
						SampleFuncType func) const
	{
		float eta;
		float cosThetaI = wo.dot(primRec.n);
		if (cosThetaI > 0) {  // same hemisphere
			// wo from outside
			eta = etaIn / etaOut;  // eta_t / eta_i
		}
		else {
			eta = etaOut / etaIn;
			cosThetaI = -cosThetaI;
		}
		const auto [totalReflection, sin2ThetaT] =
			PerfectTransmission::checkTotalReflection(eta, cosThetaI);
		if (totalReflection) {
			// sample reflection
			return func(true, 1);
		}
		else {
			const auto cosThetaT = std::sqrtf(1 - sin2ThetaT);
			const auto f = FresnelReal::fresnel(eta, cosThetaI, cosThetaT);
			return func(uSample[0] < f, f);
		}
	}
	float etaOut, etaIn;
	PerfectReflectionMaterial reflection;
	PerfectTransmissionMaterial transmission;
};

}  // namespace xd

#endif	// XD_RT_PERFECTFRESNELMATERIAL_H

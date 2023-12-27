//
// Created by Frank on 2023/8/27.
//

#ifndef XD_RT_BXDF_H
#define XD_RT_BXDF_H
#include "CoreTypes.h"
#include "MathTypes.h"
namespace xd {
struct SampleBxDFResult {
	SampleBxDFResult() = default;
	SampleBxDFResult(const SampleBxDFResult& other) = default;
	SampleBxDFResult(SampleBxDFResult&& other) noexcept = default;
	SampleBxDFResult& operator=(const SampleBxDFResult& other) = default;
	SampleBxDFResult& operator=(SampleBxDFResult&& other) noexcept = default;
	~SampleBxDFResult() = default;
	SampleBxDFResult(ColorRGB bxdf, Vector3f dir) : bxdf(std::move(bxdf)), dir(std::move(dir)) {}
	ColorRGB bxdf{};
	Vector3f dir{};
};
struct SampleBxDFPdfResult : SampleBxDFResult {
	SampleBxDFPdfResult() = default;
	SampleBxDFPdfResult(const SampleBxDFPdfResult& other) = default;
	SampleBxDFPdfResult(SampleBxDFPdfResult&& other) noexcept = default;
	SampleBxDFPdfResult& operator=(const SampleBxDFPdfResult& other) = default;
	SampleBxDFPdfResult& operator=(SampleBxDFPdfResult&& other) noexcept = default;
	~SampleBxDFPdfResult() = default;
	SampleBxDFPdfResult(ColorRGB bxdf, Vector3f dir, float pdf)
		: SampleBxDFResult(std::move(bxdf), std::move(dir)), pdf(pdf)
	{
	}

	SampleBxDFPdfResult(SampleBxDFResult other, float pdf)
		: SampleBxDFResult(std::move(other)), pdf(pdf)
	{
	}

	float pdf{0.f};
};
struct SampleDirPdfResult {
	SampleDirPdfResult() = default;
	SampleDirPdfResult(const SampleDirPdfResult& other) = default;
	SampleDirPdfResult(SampleDirPdfResult&& other) noexcept = default;
	SampleDirPdfResult& operator=(const SampleDirPdfResult& other) = default;
	SampleDirPdfResult& operator=(SampleDirPdfResult&& other) noexcept = default;
	~SampleDirPdfResult() = default;
	SampleDirPdfResult(Vector3f dir, float pdf) : dir(std::move(dir)), pdf(pdf) {}
	Vector3f dir;
	float pdf;
};
class BxDF {
public:
	virtual ~BxDF() = default;
	virtual ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const = 0;
	/**
	 * \brief sample the brdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param wo the incoming direction. Wo must lies in local frame.
	 * \return the brdf value and wo direction
	 */
	virtual SampleBxDFResult sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const = 0;
	/**
	 * \brief sample the brdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param wo the incoming direction. Wo must lies in local frame.
	 * \return the brdf value, wo direction and pdf
	 */
	virtual SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
												  const Vector3f& wo) const = 0;
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
	 * \return wo and pdf. Note that wo is in local frame.
	 */
	virtual SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
													  const Vector3f& wo) const = 0;
	virtual float getPdf(const Vector3f& wi) const = 0;
	virtual bool isDelta() const = 0;
};

}  // namespace xd
#endif	// XD_RT_BXDF_H

//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PIECEWISE1D_H
#define XD_RT_PIECEWISE1D_H
#include <span>
#include "InverseMethodDistribution.h"
namespace xd {
/**
 * \brief sample continuous piecewise in [0, 1]
 */
class PieceWise1D : public InverseMethodDistribution<1, 1> {
public:
	explicit PieceWise1D(const std::span<const float>& f);
	PieceWise1D(const float* pdf, uint32_t n);
	float sample(const float& uSample) override;
	float sampleWithPdf(const float& uSample, float& pdf) override;
	float sampleWithPdf(const float& uSample, float& pdf, uint32_t& offset);
	float getPdf(const float& sample) const override;
	float getPdfFromOffset(uint32_t offset) const;
	const std::vector<float>& getCdfs() const { return cdfs; }
	const std::vector<float>& getPdfs() const { return pdfs; }

protected:
	auto sampleHelper(const float& uSample)
	{
		struct Ret {
			uint32_t offset;
			float sample;
		} ret;
		const int n = cdfs.size();
		auto rightIt = std::lower_bound(cdfs.begin(), cdfs.end(), uSample);
		float yLeft, yRight;
		if (rightIt != cdfs.end())
			yRight = *rightIt;
		else {
			yRight = 1.f;
		}
		if (rightIt != cdfs.begin()) {
			yLeft = *std::prev(rightIt);
		}
		else
			yLeft = 0.f;
		ret.offset =
			std::clamp<uint32_t>(std::distance(cdfs.begin(), rightIt), 0u,
								 n - 1);  // distance from begin to rightIt is equivalent to 0 to
										  // left bound. We do not use leftIt here because it could
										  // be 0, which is before begin in our implementation

		const float tangent = (yRight - yLeft) * n;
		const float dx = (uSample - yLeft) / tangent;
		ret.sample = float(ret.offset) / float(n) + dx;
		return ret;
	}
	float fInt;	 // the integrated value of f
	std::vector<float> pdfs;
	std::vector<float> cdfs;
};
}  // namespace xd
#endif	// XD_RT_PIECEWISE1D_H

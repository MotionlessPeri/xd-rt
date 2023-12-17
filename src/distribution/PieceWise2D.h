//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PIECEWISE2D_H
#define XD_RT_PIECEWISE2D_H
#include "InverseMethodDistribution.h"
#include "PieceWise1D.h"
namespace xd {
class PieceWise2D : public InverseMethodDistribution<2, 2> {
public:
	PieceWise2D(const std::span<const float>& weights, uint32_t width, uint32_t height);
	RetType sample(const UniformSampleType& uSample) override;
	RetType sampleWithPdf(const UniformSampleType& uSample, float& pdf) override;
	float getPdf(const Vector2f& sample) const override;
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }

protected:
	uint32_t getIndex(uint32_t row, uint32_t col) const;
	uint32_t width;
	uint32_t height;
	std::vector<std::unique_ptr<PieceWise1D>> conditionals;	 // p(u|v)
	std::unique_ptr<PieceWise1D> marginalV;					 // p(v)
};
}  // namespace xd
#endif	// XD_RT_PIECEWISE2D_H

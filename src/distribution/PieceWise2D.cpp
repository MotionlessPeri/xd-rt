//
// Created by Frank on 2023/9/22.
//
#include <numeric>
#include "Distribution.h"
using namespace xd;
PieceWise2D::PieceWise2D(const std::vector<float>& weights, uint32_t width, uint32_t height)
	: width(width), height(height)
{
	std::vector<float> sumV(height, 0.f);
	for (auto row = 0u; row < height; ++row) {
		for (auto col = 0u; col < width; ++col) {
			const auto index = row * width + col;
			sumV[row] += weights[index];
		}
		conditionals.emplace_back(&weights[row * width], width);
	}
	marginalV = PieceWise1D{sumV};
	float allSum = std::accumulate(sumV.cbegin(), sumV.cend(), 0.f);
}
Vector2f PieceWise2D::operator()() const
{
	float dummyPdf;
	return (*this)(dummyPdf);
}
Vector2f PieceWise2D::operator()(float& pdf) const
{
	uint32_t vIdx, uIdx;
	float vPdf, uPdf;
	const auto v = marginalV(vPdf, vIdx);			// p(v)
	const auto u = conditionals[vIdx](uPdf, uIdx);	// p(u|v)
	pdf = uPdf * vPdf;
	return {u, v};
}
float PieceWise2D::getPdf(const Vector2f& sample) const
{
	const float v = sample.y();
	const float u = sample.x();
	const float dv = 1.f / height;
	const float du = 1.f / width;
	const uint32_t vIdx = std::clamp((uint32_t)std::floorf(v / dv), 0u, height - 1);
	return marginalV.getPdf(v) * conditionals[vIdx].getPdf(u);
}
uint32_t PieceWise2D::getIndex(uint32_t row, uint32_t col) const
{
	return row * width + col;
}

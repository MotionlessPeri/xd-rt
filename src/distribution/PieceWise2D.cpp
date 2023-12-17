//
// Created by Frank on 2023/9/22.
//
#include "PieceWise2D.h"
using namespace xd;
PieceWise2D::PieceWise2D(const std::span<const float>& weights, uint32_t width, uint32_t height)
	: width(width), height(height)
{
	std::vector<float> sumV(height, 0.f);
	for (auto row = 0u; row < height; ++row) {
		for (auto col = 0u; col < width; ++col) {
			const auto index = row * width + col;
			sumV[row] += weights[index];
		}
		conditionals.emplace_back(std::make_unique<PieceWise1D>(&weights[row * width], width));
	}
	marginalV = std::make_unique<PieceWise1D>(sumV);
}

float PieceWise2D::getPdf(const Vector2f& sample) const
{
	const float v = sample.y();
	const float u = sample.x();
	const float dv = 1.f / height;
	const float du = 1.f / width;
	const uint32_t vIdx = std::clamp((uint32_t)std::floorf(v / dv), 0u, height - 1);
	return marginalV->getPdf(v) * conditionals[vIdx]->getPdf(u);
}
uint32_t PieceWise2D::getIndex(uint32_t row, uint32_t col) const
{
	return row * width + col;
}
Vector2f PieceWise2D::sample(const Vector2f& uSample)
{
	uint32_t vIdx, uIdx;
	float vPdf, uPdf;
	const auto v = marginalV->sampleWithPdf(uSample(0), vPdf, vIdx);  // p(v)
	const auto u = conditionals[vIdx]->sample(uSample(1));			  // p(u|v)
	return {u, v};
}
Vector2f PieceWise2D::sampleWithPdf(const Vector2f& uSample, float& pdf)
{
	uint32_t vIdx, uIdx;
	float vPdf, uPdf;
	const auto v = marginalV->sampleWithPdf(uSample(0), vPdf, vIdx);		   // p(v)
	const auto u = conditionals[vIdx]->sampleWithPdf(uSample(1), uPdf, uIdx);  // p(u|v)
	pdf = uPdf * vPdf;
	return {u, v};
}

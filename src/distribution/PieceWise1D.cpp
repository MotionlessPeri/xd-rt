//
// Created by Frank on 2023/9/22.
//
#include <numeric>
#include "Distribution.h"

using namespace xd;
PieceWise1D::PieceWise1D(const std::vector<float>& f) : InverseMethodDistribution<1, 1>(), pdfs(f)
{
	sum = std::accumulate(f.cbegin(), f.cend(), 0.f);
	for (auto& val : pdfs)
		val /= sum;
	cdfs.reserve(f.size());
	cdfs.emplace_back(f.front() / sum);
	for (auto it = f.cbegin() + 1; it != f.cend(); ++it) {
		cdfs.emplace_back(cdfs.back() + *it / sum);
	}
}
PieceWise1D::PieceWise1D(const float* pdf, const uint32_t n) : PieceWise1D({pdf, pdf + n}) {}
float PieceWise1D::getPdf(const float& sample) const
{
	const float delta = 1.f / pdfs.size();
	const uint32_t offset = std::clamp<uint32_t>(sample / delta, 0, pdfs.size() - 1);
	return pdfs[offset];
}
float PieceWise1D::sample(const float& uSample)
{
	return sampleHelper(uSample).sample;
}
float PieceWise1D::sampleWithPdf(const float& uSample, float& pdf)
{
	const auto res = sampleHelper(uSample);
	pdf = this->pdfs[res.offset];
	return res.sample;
}
float PieceWise1D::sampleWithPdf(const float& uSample, float& pdf, uint32_t& offset)
{
	const auto res = sampleHelper(uSample);
	offset = res.offset;
	pdf = this->pdfs[offset];
	return res.sample;
}

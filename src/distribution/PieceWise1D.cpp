//
// Created by Frank on 2023/9/22.
//
#include <numeric>
#include "Distribution.h"
#include "ranges"

using namespace xd;

PieceWise1D::PieceWise1D(const std::span<const float>& f)
{
	const int n = f.size();
	const float nInv = 1.f / n;
	cdfs.reserve(n);

	const auto fdxView = f | std::views::transform([&](float fVal) { return fVal * nInv; });
	std::partial_sum(fdxView.begin(), fdxView.end(), std::back_inserter(cdfs));
	fInt = cdfs.back();
	std::ranges::for_each(cdfs, [&](float& cdfVal) { cdfVal /= fInt; });

	// Note: in c++23 we can use std::ranges::to<std::vector<float>>()
	const auto pdfNormedView = (f | std::views::transform([&](float fVal) { return fVal / fInt; }));
	pdfs = {pdfNormedView.begin(), pdfNormedView.end()};
}

PieceWise1D::PieceWise1D(const float* pdf, uint32_t n) : PieceWise1D(std::span(pdf, n)) {}

float PieceWise1D::getPdf(const float& sample) const
{
	const float delta = 1.f / pdfs.size();
	const uint32_t offset = std::clamp<uint32_t>(sample / delta, 0, pdfs.size() - 1);
	return getPdfFromOffset(offset);
}

float PieceWise1D::getPdfFromOffset(uint32_t offset) const
{
	return pdfs[offset];
}

float PieceWise1D::sample(const float& uSample)
{
	return sampleHelper(uSample).sample;
}
float PieceWise1D::sampleWithPdf(const float& uSample, float& pdf)
{
	const auto res = sampleHelper(uSample);
	pdf = getPdfFromOffset(res.offset);
	return res.sample;
}
float PieceWise1D::sampleWithPdf(const float& uSample, float& pdf, uint32_t& offset)
{
	const auto res = sampleHelper(uSample);
	offset = res.offset;
	pdf = getPdfFromOffset(res.offset);
	return res.sample;
}

//
// Created by Frank on 2023/9/22.
//
#include <numeric>
#include "Distribution.h"

using namespace xd;
PieceWise1D::PieceWise1D(const std::vector<float>& f) : pdfs(f)
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
float PieceWise1D::operator()() const
{
	uint32_t dummy;
	return (*this)(dummy);
}
float PieceWise1D::operator()(uint32_t& offset) const
{
	float dummyPdf;
	return (*this)(dummyPdf, offset);
}
float PieceWise1D::operator()(float& pdf) const
{
	uint32_t dummyOffset;
	return (*this)(pdf, dummyOffset);
}
float PieceWise1D::operator()(float& pdf, uint32_t& offset) const
{
	// TODO: consider how to access uniform samples
	// pbrt set the uniform sample as a parameter
	// what should we do? Do we need a global sampler?
	// If so, how do we implement it? Does it need to be thread-safe?
	static std::random_device rd;
	static std::mt19937 gen(rd());	// Standard mersenne_twister_engine seeded with rd()
	// static std::default_random_engine gen(rd());
	static std::uniform_real_distribution<float> ksai(0.f, 1.f);
	const float sample = ksai(gen);
	auto rightIt = std::lower_bound(cdfs.begin(), cdfs.end(), sample);
	float left, right;
	if (rightIt != cdfs.end())
		right = *rightIt;
	else {
		right = 1.f;
	}
	if (rightIt != cdfs.begin()) {
		left = *std::prev(rightIt);
	}
	else
		left = 0.f;
	const float delta = 1.f / this->pdfs.size();
	offset = std::clamp<uint32_t>(std::distance(cdfs.begin(), rightIt), 0u, cdfs.size() - 1);
	pdf = this->pdfs[offset];
	const float xOffset = delta * offset;
	const float dy = sample - left;
	const float dx = dy / (right - left) * delta;
	return xOffset + dx;
}
float PieceWise1D::getPdf(const float& sample) const
{
	const float delta = 1.f / pdfs.size();
	const uint32_t offset = sample / delta;
	return pdfs[offset];
}

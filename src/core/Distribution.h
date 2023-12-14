//
// Created by Frank on 2023/8/28.
//

#ifndef XD_RT_DISTRIBUTION_H
#define XD_RT_DISTRIBUTION_H
#include <random>
#include <ranges>
#include "MathType.h"
namespace xd {
template <uint32_t N>
class Distribution {
public:
	virtual ~Distribution() = default;
	virtual VectorNf<N> sample() = 0;
	virtual VectorNf<N> sampleWithPdf(float& pdf) = 0;
	virtual float getPdf(const VectorNf<N>& sample) const = 0;
};

typedef Distribution<3> Distribution3f;
typedef Distribution<2> Distribution2f;
typedef Distribution<1> Distributionf;

template <uint32_t N>
class UniformDistribution : Distribution<N> {
public:
	explicit UniformDistribution()
	{
		for (auto& gen : generators) {
			gen = std::mt19937{rd()};
		}
	}
	explicit UniformDistribution(int seed)
	{
		std::seed_seq seq{std::initializer_list<int>{seed}};
		std::array<unsigned int, N> seeds;
		seq.generate(seeds.begin(), seeds.end());
		for (auto i : std::views::iota(0u, N)) {
			generators[i] = std::mt19937{seeds[i]};
		}
	}
	VectorNf<N> sample() override
	{
		VectorNf<N> res;
		if constexpr (N != 1) {
			for (auto i = 0u; i < N; ++i) {
				res(i) = dis(generators[i]);
			}
		}
		else {
			res = dis(generators[0]);
		}

		return res;
	}
	VectorNf<N> sampleWithPdf(float& pdf) override
	{
		pdf = 1.f;
		return sample();
	}
	float getPdf(const VectorNf<N>& sample) const override { return 1.f; }

protected:
	std::random_device rd;
	std::array<std::mt19937, N> generators;
	std::uniform_real_distribution<float> dis{0, 1.f - 1e-4f};
};

template <uint32_t N>
class UniformDiscreteDistribution {
public:
	explicit UniformDiscreteDistribution(int lower, int upper, int seed) : dis(lower, upper)
	{
		std::seed_seq seq{std::initializer_list<int>{seed}};
		std::array<unsigned int, N> seeds;
		seq.generate(seeds.begin(), seeds.end());
		for (auto i : std::views::iota(0u, N)) {
			generators[i] = std::mt19937{seeds[i]};
		}
	}

	VectorNi<N> sample()
	{
		VectorNi<N> res;
		if constexpr (N != 1) {
			for (auto i = 0u; i < N; ++i) {
				res(i) = dis(generators[i]);
			}
		}
		else {
			res = dis(generators[0]);
		}

		return res;
	}

protected:
	std::array<std::mt19937, N> generators;
	std::uniform_int_distribution<int> dis;
};

template <uint32_t RetDim, uint32_t UniformSampleDim>
class InverseMethodDistribution : public Distribution<RetDim> {
public:
	typedef VectorNf<UniformSampleDim> UniformSampleType;
	typedef VectorNf<RetDim> RetType;
	InverseMethodDistribution() : rng(std::make_unique<UniformDistribution<UniformSampleDim>>()) {}
	virtual RetType sample(const UniformSampleType& uSample) = 0;
	virtual RetType sampleWithPdf(const UniformSampleType& uSample, float& pdf) = 0;
	RetType sample() override { return sample(rng->sample()); }
	RetType sampleWithPdf(float& pdf) override { return sampleWithPdf(rng->sample(), pdf); }

protected:
	std::unique_ptr<UniformDistribution<UniformSampleDim>> rng{};
};
/**
 * A uniform distribution on a hemisphere which center at (0,0,1)
 */
class UniformHemisphere : public InverseMethodDistribution<3, 2> {
public:
	RetType sample(const UniformSampleType& uSample) override;
	RetType sampleWithPdf(const UniformSampleType& uSample, float& pdf) override;
	float getPdf(const Vector3f& sample) const override;
};

class CosineHemisphere : public InverseMethodDistribution<3, 2> {
public:
	RetType sample(const UniformSampleType& uSample) override;
	RetType sampleWithPdf(const UniformSampleType& uSample, float& pdf) override;
	float getPdf(const Vector3f& sample) const override;
};

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
#endif	// XD_RT_DISTRIBUTION_H

//
// Created by Frank on 2023/8/28.
//

#ifndef XD_RT_DISTRIBUTION_H
#define XD_RT_DISTRIBUTION_H
#include <random>

#include "MathType.h"
namespace xd {
template <typename VecType>
class Distribution {
public:
	virtual ~Distribution() = default;
	virtual VecType operator()() const = 0;
	virtual VecType operator()(float& pdf) const = 0;
	virtual float getPdf(const VecType& sample) const = 0;
};

typedef Distribution<Vector3f> Distribution3f;
typedef Distribution<Vector2f> Distribution2f;
typedef Distribution<float> Distributionf;

/**
 * A uniform distribution on a hemisphere which center at (0,0,1)
 */
class UniformHemisphere : public Distribution3f {
public:
	Vector3f operator()() const override;
	Vector3f operator()(float& pdf) const override;
	float getPdf(const Vector3f& sample) const override;
};

class CosineHemisphere : public Distribution3f {
public:
	Vector3f operator()() const override;
	Vector3f operator()(float& pdf) const override;
	float getPdf(const Vector3f& sample) const override;
};

class PieceWise1D : public Distributionf {
public:
	PieceWise1D() = default;
	explicit PieceWise1D(const std::vector<float>& f);
	PieceWise1D(const float* pdf, const uint32_t n);
	float operator()() const override;
	float operator()(float& pdf) const override;
	float operator()(uint32_t& offset) const;
	float operator()(float& pdf, uint32_t& offset) const;
	float getPdf(const float& sample) const override;
	const std::vector<float>& getCdfs() const { return cdfs; }
	const std::vector<float>& getPdfs() const { return pdfs; }

protected:
	float sum;
	std::vector<float> pdfs;
	std::vector<float> cdfs;
};

class PieceWise2D : public Distribution2f {
public:
	PieceWise2D(const std::vector<float>& weights, uint32_t width, uint32_t height);
	Vector2f operator()() const override;
	Vector2f operator()(float& pdf) const override;
	float getPdf(const Vector2f& sample) const override;
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }

protected:
	uint32_t getIndex(uint32_t row, uint32_t col) const;
	uint32_t width;
	uint32_t height;
	std::vector<PieceWise1D> conditionals;	// p(u|v)
	PieceWise1D marginalV;					// p(v)
};
}  // namespace xd
#endif	// XD_RT_DISTRIBUTION_H

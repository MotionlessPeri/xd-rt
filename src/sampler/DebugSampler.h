//
// Created by Frank on 2023/12/3.
//

#ifndef XD_RT_DEBUGSAMPLER_H
#define XD_RT_DEBUGSAMPLER_H
#include "Sampler.h"
static constexpr std::size_t MAX_SIZE = 1024;
namespace xd {
class DebugSampler : public Sampler {
public:
	explicit DebugSampler(int samplerPerPixel);
	void request1DArray(int n) override;
	void request2DArray(int n) override;
	const std::span<const float> get1DArray(int n) override;
	const std::span<const Vector2f> get2DArray(int n) override;
	float sample1D() override;
	Vector2f sample2D() override;
	std::unique_ptr<Sampler> clone(int seed) const override;

protected:
	// This member is not inlined because we wanna use this ctor:
	// std::vector<>(size_type count,const T& value, const Allocator& alloc = Allocator() )
	// which will be deduced to
	// std::vector<>(std::initializer_list<T> init, const Allocator& alloc = Allocator())
	// using brace init here
	const static std::vector<float> array1d;
	inline const static std::vector<Vector2f> array2d{MAX_SIZE, Vector2f{0.5f, 0.5f}};
};
}  // namespace xd

#endif	// XD_RT_DEBUGSAMPLER_H

//
// Created by Frank on 2023/8/28.
//

#ifndef XD_RT_DISTRIBUTION_H
#define XD_RT_DISTRIBUTION_H
#include "MathType.h"
namespace xd {
template <typename VecType>
class Distribution {
public:
	virtual ~Distribution() = default;
	virtual VecType operator()() const = 0;
};

typedef Distribution<Vector3f> Distribution3f;

/**
 * A uniform distribution on a hemisphere which center at (0,0,1)
 */
class UniformHemisphere : public Distribution3f {
public:
	Vector3f operator()() const override;
};
}  // namespace xd
#endif	// XD_RT_DISTRIBUTION_H

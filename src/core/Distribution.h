//
// Created by Frank on 2023/8/28.
//

#ifndef XD_RT_DISTRIBUTION_H
#define XD_RT_DISTRIBUTION_H

#include "MathTypes.h"
namespace xd {
template <uint32_t N>
class Distribution {
public:
	virtual ~Distribution() = default;
	virtual VectorNf<N> sample() = 0;
	virtual VectorNf<N> sampleWithPdf(float& pdf) = 0;
	virtual float getPdf(const VectorNf<N>& sample) const = 0;
};

}  // namespace xd
#endif	// XD_RT_DISTRIBUTION_H

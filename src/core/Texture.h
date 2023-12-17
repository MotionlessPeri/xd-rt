//
// Created by Frank on 2023/8/31.
//

#ifndef XD_RT_TEXTURE_H
#define XD_RT_TEXTURE_H
#include "MathUtil.h"
namespace xd {
template <typename ReturnType, typename SampleType>
class Texture {
public:
	virtual ReturnType sample(const SampleType& sample) = 0;
};

}  // namespace xd
#endif	// XD_RT_TEXTURE_H

//
// Created by Frank on 2024/1/30.
//

#ifndef XD_RT_SHADERTRAITS_H
#define XD_RT_SHADERTRAITS_H
namespace xd {
template <typename T>
struct ShaderTraits {
	using typename T::ShaderParams;
};

}  // namespace xd
#endif	// XD_RT_SHADERTRAITS_H

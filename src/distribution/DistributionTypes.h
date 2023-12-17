//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_DISTRIBUTIONTYPES_H
#define XD_RT_DISTRIBUTIONTYPES_H

#include <cstdint>
namespace xd {
template <uint32_t N>
class Distribution;
typedef Distribution<3> Distribution3f;
typedef Distribution<2> Distribution2f;
typedef Distribution<1> Distributionf;
template <uint32_t N>
class UniformDistribution;
template <uint32_t N>
class DiscreteUniformDistribution;
template <uint32_t RetDim, uint32_t UniformSampleDim>
class InverseMethodDistribution;

class CosineHemisphere;
class UniformHemisphere;
class PieceWise1D;
class PieceWise2D;
}  // namespace xd
#endif	// XD_RT_DISTRIBUTIONTYPES_H

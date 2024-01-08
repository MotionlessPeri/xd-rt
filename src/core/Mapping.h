//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_MAPPING_H
#define XD_RT_MAPPING_H
#include "Contexts.h"
#include "MathTypes.h"
namespace xd {
template <uint32_t OutputDim>
class Mapping {
public:
	using OutputType = VectorNf<OutputDim>;
	Mapping() = default;
	Mapping(const Mapping& other) = default;
	Mapping(Mapping&& other) noexcept = default;
	Mapping& operator=(const Mapping& other) = default;
	Mapping& operator=(Mapping&& other) noexcept = default;
	virtual ~Mapping() = default;
	/**
	 * \brief mapping a set of coords(mostly from geom representation or volume representation) to
	 * another space(mostly texture space)
	 * \param coords the coords of input point. The coords lies in [0,1]^N
	 * \return the mapped coords
	 */
	virtual OutputType map(const TextureEvalContext& coords) const = 0;
};

using Mapping2D = Mapping<2>;
using Mapping3D = Mapping<3>;
}  // namespace xd
#endif	// XD_RT_MAPPING_H

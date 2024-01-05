//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_MAPPING_H
#define XD_RT_MAPPING_H
#include "MathTypes.h"
namespace xd {
template <uint32_t InputDim, uint32_t OutputDim>
class Mapping {
public:
	using InputType = VectorNf<InputDim>;
	using OutputType = VectorNf<OutputDim>;
	virtual ~Mapping() = default;
	/**
	 * \brief mapping a set of coords(mostly from geom representation or volume representation) to
	 * another space(mostly texture space)
	 * \param coords the coords of input point. The coords lies in [0,1]^N
	 * \return the mapped coords
	 */
	virtual OutputType map(const InputType& coords) const = 0;
};
}  // namespace xd
#endif	// XD_RT_MAPPING_H

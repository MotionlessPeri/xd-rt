//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_EQUIRECTANGULARMAPPING_H
#define XD_RT_EQUIRECTANGULARMAPPING_H
#include "Mapping.h"
namespace xd {

class EquirectangularMapping : public Mapping2D {
public:
	/**
	 * \brief mapping a spherical direction to an (s, t) representation
	 * \param ctx a texture evaluate context that holds all information to sample a texture value
	 * \return the mapped coords
	 */
	OutputType map(const TextureEvalContext& ctx) const override;
};

}  // namespace xd

#endif	// XD_RT_EQUIRECTANGULARMAPPING_H

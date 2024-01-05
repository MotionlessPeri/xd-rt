//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_EQUIRECTANGULARMAPPING_H
#define XD_RT_EQUIRECTANGULARMAPPING_H
#include "Mapping.h"
namespace xd {

class EquirectangularMapping : public Mapping<3, 2> {
public:
	/**
	 * \brief mapping a spherical direction to an (s, t) representation
	 * \param dir the coords of input point. Note that the coords must be normed(a direction on
	 * S2)
	 * \return the mapped coords
	 */
	OutputType map(const InputType& dir) const override;
};

}  // namespace xd

#endif	// XD_RT_EQUIRECTANGULARMAPPING_H

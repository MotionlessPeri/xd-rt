//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_UVMAPPING_H
#define XD_RT_UVMAPPING_H
#include "Mapping.h"
namespace xd {

class UVMapping : public Mapping<2, 2> {
public:
	UVMapping() = default;
	/**
	 * \brief build UVMapping with respect to transform params
	 * \param sx scale in x direction
	 * \param sy scale in y direction
	 * \param tx translation in x direction
	 * \param ty translation in y direction
	 * \param r rotation angle from pos-x direction in radians
	 */
	UVMapping(float sx, float sy, float tx, float ty, float r);

	OutputType map(const InputType& coords) const override;

protected:
	float sx = 1, sy = 1, tx = 0, ty = 0, r = 0;
};

}  // namespace xd

#endif	// XD_RT_UVMAPPING_H

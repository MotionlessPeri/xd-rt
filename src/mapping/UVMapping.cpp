//
// Created by Frank on 2024/1/5.
//

#include "UVMapping.h"
using namespace xd;
UVMapping::UVMapping(float sx, float sy, float tx, float ty, float r)
	: sx(sx), sy(sy), tx(tx), ty(ty), r(r)
{
}

Mapping<2, 2>::OutputType UVMapping::map(const InputType& coords) const
{
	auto res = coords;
	res = Eigen::Rotation2Df{r} * res;
	res = Eigen::Scaling(sx, sy) * res;
	res += Vector2f{tx, ty};
	return res;
}

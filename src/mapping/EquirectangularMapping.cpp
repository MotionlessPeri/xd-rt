//
// Created by Frank on 2024/1/5.
//

#include "EquirectangularMapping.h"
#include "MathUtil.h"
using namespace xd;

Mapping<2>::OutputType EquirectangularMapping::map(const TextureEvalContext& ctx) const
{
	return getSphereUV(ctx.p);
}

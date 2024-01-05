//
// Created by Frank on 2024/1/5.
//

#include "EquirectangularMapping.h"
#include "MathUtil.h"
using namespace xd;
EquirectangularMapping::OutputType EquirectangularMapping::map(const InputType& dir) const
{
	return getSphereUV(dir);
}

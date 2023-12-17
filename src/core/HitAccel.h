//
// Created by Frank on 2023/9/11.
//

#ifndef XD_RT_HITACCEL_H
#define XD_RT_HITACCEL_H
#include "Hitable.h"
namespace xd {
/**
 * @brief Hit acceleration structure interface
 *
 * This class represents hit accelerate structure. It inherits HitAggregate, which means any
 * HitAccel must provide both closest hit query and any hit query.
 */
class HitAccel : public HitAggregate {};

}  // namespace xd
#endif	// XD_RT_HITACCEL_H

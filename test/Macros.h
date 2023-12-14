//
// Created by Frank on 2023/12/10.
//

#ifndef XD_RT_MACROS_H
#define XD_RT_MACROS_H
#include "oneapi/tbb.h"
#define TBB_SERIAL                                                                                 \
	oneapi::tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism, \
											 1);
#endif	// XD_RT_MACROS_H

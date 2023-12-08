//
// Created by Frank on 2023/8/16.
//

#include "Ray.h"
using namespace xd;

Ray::Ray(const Vector3f& o, const Vector3f& d) : o(o), oError(0, 0, 0), d(d), dError(0, 0, 0) {}

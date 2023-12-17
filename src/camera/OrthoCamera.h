//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_ORTHOCAMERA_H
#define XD_RT_ORTHOCAMERA_H
#include "Camera.h"
namespace xd {
class OrthoCamera : public Camera {
public:
	OrthoCamera(const std::shared_ptr<Film>& film);
	Ray generateRay(const Vector2f& sample) const override;
};
}  // namespace xd
#endif	// XD_RT_ORTHOCAMERA_H

//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_CAMERA_H
#define XD_RT_CAMERA_H

#include <memory>

#include "CoreTypes.h"
#include "MathType.h"

namespace xd {
class Camera {
public:
	Camera(const std::shared_ptr<Film>& film);
	virtual Ray generateRay(const Vector2f& sample) const = 0;
	std::shared_ptr<Film> getFilm() const { return film; }

protected:
	std::shared_ptr<Film> film;
};

class PerspCamera : public Camera {
public:
	PerspCamera(const Vector3f& position, const std::shared_ptr<Film>& film);
	Ray generateRay(const Vector2f& sample) const override;

protected:
	Vector3f pos;
};

class OrthoCamera : public Camera {
public:
	OrthoCamera(const std::shared_ptr<Film>& film);
	Ray generateRay(const Vector2f& sample) const override;
};
}  // namespace xd
#endif	// XD_RT_CAMERA_H

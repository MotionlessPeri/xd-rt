//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_CAMERA_H
#define XD_RT_CAMERA_H

#include <memory>

#include "Film.h"
#include "MathTypes.h"
#include "Ray.h"

namespace xd {
class Camera {
public:
	explicit Camera(const std::shared_ptr<Film>& pFilm) : film(pFilm){};
	[[nodiscard]] virtual Ray generateRay(const Vector2f& sample) const = 0;
	[[nodiscard]] std::shared_ptr<Film> getFilm() const { return film; }

protected:
	std::shared_ptr<Film> film;
};

}  // namespace xd
#endif	// XD_RT_CAMERA_H

//
// Created by Frank on 2023/12/17.
//
#include "PerspCamera.h"
#include "Film.h"
#include "Ray.h"
using namespace xd;
PerspCamera::PerspCamera(const Vector3f& position, const std::shared_ptr<Film>& film)
	: Camera(film), pos(position)
{
}

Ray PerspCamera::generateRay(const Vector2f& sample) const
{
	Vector3f origin = pos;
	Vector3f target = film->getWorldCoordsFromSample(sample);
	return {origin, (target - origin).normalized()};
}

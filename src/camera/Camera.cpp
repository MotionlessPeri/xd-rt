//
// Created by Frank on 2023/8/17.
//
#include "Camera.h"
#include "Film.h"
#include "Ray.h"
using namespace xd;

Camera::Camera(const std::shared_ptr<Film>& pFilm) : film(pFilm) {}

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
OrthoCamera::OrthoCamera(const std::shared_ptr<Film>& film) : Camera(film) {}

Ray OrthoCamera::generateRay(const Vector2f& sample) const
{
	return {film->getWorldCoordsFromSample(sample), film->getTowards()};
}

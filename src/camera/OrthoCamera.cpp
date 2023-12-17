//
// Created by Frank on 2023/12/17.
//
#include "OrthoCamera.h"
#include "Film.h"
#include "Ray.h"
using namespace xd;
OrthoCamera::OrthoCamera(const std::shared_ptr<Film>& film) : Camera(film) {}

Ray OrthoCamera::generateRay(const Vector2f& sample) const
{
	return {film->getWorldCoordsFromSample(sample), film->getTowards()};
}

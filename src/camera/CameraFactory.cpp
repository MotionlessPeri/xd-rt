//
// Created by Frank on 2023/8/22.
//
#include "CameraFactory.h"
#include "Film.h"

using namespace xd;

std::shared_ptr<OrthoCamera> CameraFactory::createOrthoCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float rightNorm,
															  float upNorm,
															  uint32_t width,
															  uint32_t height)
{
	return createOrthoCamera(position, target, upUnit, rightNorm, upNorm, width, height, {0, 0},
							 {width - 1, height - 1});
}

std::shared_ptr<OrthoCamera> CameraFactory::createOrthoCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float rightNorm,
															  float upNorm,
															  uint32_t width,
															  uint32_t height,
															  const Vector2i& croppedTopLeft,
															  const Vector2i& croppedBottomRight)
{
	const Vector3f up = upNorm * upUnit;
	const Vector3f direction = (target - position).normalized();
	const Vector3f right = (direction.cross(upUnit)) * rightNorm;
	auto film = std::make_shared<Film>(position, right, up, width, height, croppedTopLeft,
									   croppedBottomRight);
	auto camera = std::make_shared<OrthoCamera>(film);
	return camera;
}

std::shared_ptr<OrthoCamera> CameraFactory::createOrthoCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float rightNorm,
															  float upNorm,
															  uint32_t width)
{
	const uint32_t height = std::floorf(float(width) / rightNorm * upNorm);
	return CameraFactory::createOrthoCamera(position, target, upUnit, rightNorm, upNorm, width,
											height);
}

std::shared_ptr<OrthoCamera> CameraFactory::createOrthoCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float rightNorm,
															  float upNorm,
															  uint32_t width,
															  const Vector2i& croppedTopLeft,
															  const Vector2i& croppedBottomRight)
{
	const uint32_t height = std::floorf(float(width) / rightNorm * upNorm);
	return CameraFactory::createOrthoCamera(position, target, upUnit, rightNorm, upNorm, width,
											height, croppedTopLeft, croppedBottomRight);
}

std::shared_ptr<PerspCamera> CameraFactory::createPerspCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float verticalFov,
															  float aspect,
															  uint32_t width,
															  uint32_t height,
															  const Vector2i& croppedTopLeft,
															  const Vector2i& croppedBottomRight)
{
	const Vector3f posToTarget = target - position;
	const Vector3f direction = posToTarget.normalized();
	const float distance = posToTarget.norm();
	const float upNorm = distance * std::tan(verticalFov / 2.f);
	const float rightNorm = upNorm * aspect;
	const Vector3f up = upNorm * upUnit;
	const Vector3f right = direction.cross(upUnit) * rightNorm;
	auto film = std::make_shared<Film>(target, right, up, width, height, croppedTopLeft,
									   croppedBottomRight);
	auto camera = std::make_shared<PerspCamera>(position, film);
	return camera;
}

std::shared_ptr<PerspCamera> CameraFactory::createPerspCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float verticalFov,
															  float aspect,
															  uint32_t width,
															  uint32_t height)
{
	return createPerspCamera(position, target, upUnit, verticalFov, aspect, width, height, {0, 0},
							 {width - 1, height - 1});
}

std::shared_ptr<PerspCamera> CameraFactory::createPerspCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float verticalFov,
															  float aspect,
															  uint32_t width,
															  const Vector2i& croppedTopLeft,
															  const Vector2i& croppedBottomRight)
{
	const uint32_t height = width / aspect;
	return createPerspCamera(position, target, upUnit, verticalFov, aspect, width, height,
							 croppedTopLeft, croppedBottomRight);
}

std::shared_ptr<PerspCamera> CameraFactory::createPerspCamera(const Vector3f& position,
															  const Vector3f& target,
															  const Vector3f& upUnit,
															  float verticalFov,
															  float aspect,
															  uint32_t width)
{
	const uint32_t height = width / aspect;
	return createPerspCamera(position, target, upUnit, verticalFov, aspect, width, height);
}

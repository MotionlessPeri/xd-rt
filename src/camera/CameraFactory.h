//
// Created by Frank on 2023/8/22.
//

#ifndef XD_RT_CAMERAFACTORY_H
#define XD_RT_CAMERAFACTORY_H
#include "MathTypes.h"
#include "OrthoCamera.h"
#include "PerspCamera.h"
namespace xd {
class CameraFactory {
public:
	static std::shared_ptr<OrthoCamera> createOrthoCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float rightNorm,
														  float upNorm,
														  uint32_t width,
														  uint32_t height);
	static std::shared_ptr<OrthoCamera> createOrthoCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float rightNorm,
														  float upNorm,
														  uint32_t width,
														  uint32_t height,
														  const Vector2i& croppedTopLeft,
														  const Vector2i& croppedBottomRight);
	static std::shared_ptr<OrthoCamera> createOrthoCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float rightNorm,
														  float upNorm,
														  uint32_t width);
	static std::shared_ptr<OrthoCamera> createOrthoCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float rightNorm,
														  float upNorm,
														  uint32_t width,
														  const Vector2i& croppedTopLeft,
														  const Vector2i& croppedBottomRight);
	static std::shared_ptr<PerspCamera> createPerspCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float verticalFov,
														  float aspect,
														  uint32_t width,
														  uint32_t height,
														  const Vector2i& croppedTopLeft,
														  const Vector2i& croppedBottomRight);
	static std::shared_ptr<PerspCamera> createPerspCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float verticalFov,
														  float aspect,
														  uint32_t width,
														  uint32_t height);
	static std::shared_ptr<PerspCamera> createPerspCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float verticalFov,
														  float aspect,
														  uint32_t width,
														  const Vector2i& croppedTopLeft,
														  const Vector2i& croppedBottomRight);
	static std::shared_ptr<PerspCamera> createPerspCamera(const Vector3f& position,
														  const Vector3f& target,
														  const Vector3f& upUnit,
														  float verticalFov,
														  float aspect,
														  uint32_t width);

protected:
};
}  // namespace xd
#endif	// XD_RT_CAMERAFACTORY_H

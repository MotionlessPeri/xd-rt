//
// Created by Frank on 2024/1/20.
//

#ifndef XD_RT_CAMERAHANDLER_H
#define XD_RT_CAMERAHANDLER_H
#include "MathTypes.h"
namespace xd {

class CameraHandler {
	CameraHandler(Vector3f rotation, Vector3f position)
		: rotation(std::move(rotation)), position(std::move(position))
	{
	}

public:
	void rotate(const Vector3f& hpr) { rotation += hpr * rotationSpeed; }
	void translate(const Vector3f& delta) { position += delta * movementSpeed; }
	Matrix4f getViewMatrix() const
	{
		Transform rot = Transform{Eigen::AngleAxisf{rotation.x(), Vector3f::UnitX()} *
								  Eigen::AngleAxisf{rotation.y(), Vector3f::UnitY()} *
								  Eigen::AngleAxisf{rotation.z(), Vector3f::UnitZ()}};
		Transform trans = Transform{Eigen::Translation3f{position}};
		return (rot * trans).matrix();
	}

private:
	Vector3f rotation;
	Vector3f position;
	float rotationSpeed = 1.f;
	float movementSpeed = 1.f;
};

}  // namespace xd

#endif	// XD_RT_CAMERAHANDLER_H

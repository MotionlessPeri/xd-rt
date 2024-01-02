//
// Created by Frank on 2024/1/2.
//

#ifndef XD_RT_TESTSCENES_H
#define XD_RT_TESTSCENES_H
#include "Material.h"
#include "Model.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "camera/OrthoCamera.h"
#include "camera/PerspCamera.h"
#include "light/DomeLight.h"
#include "model/Sphere.h"
using namespace xd;
class SceneFactory {
public:
	struct RetType {
		std::shared_ptr<Scene> scene;
		std::shared_ptr<Camera> cam;
	};
	static RetType SingleSphereScene(const std::shared_ptr<PhysicalPlausibleMaterial>& material,
									 uint32_t width = 1000u,
									 uint32_t height = 1000u)
	{
		const auto sphere = std::make_shared<Sphere>(1.f);
		SceneBuilder sb;
		sb.addPrimitive(std::make_shared<Primitive>(sphere, material));
		static const auto dome = std::make_shared<DomeLight>(R"(D:\dome.hdr)");
		sb.addEnvironment(dome);
		sb.setHitSolverType(HitSolverType::NAIVE);

		const Vector3f center = Vector3f{0, 1.7f, 0};
		const Vector3f z{0, 0, 1};
		const Vector3f target{0, 0, 0};
		const Vector3f towards = (target - center).normalized();
		const Vector3f right = towards.cross(z).normalized();
		const Vector3f up = right.cross(towards);
		constexpr auto verticalFov = toRadians(90.f);
		const auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(),
														  verticalFov, 1, width, height);

		return {sb.build(), cam};
	}
};
#endif	// XD_RT_TESTSCENES_H

//
// Created by Frank on 2023/11/29.
//

#ifndef XD_RT_SCENEBUILDER_H
#define XD_RT_SCENEBUILDER_H

#include "CoreTypes.h"
#include "Enums.h"
#include "Light.h"

namespace xd {
class SceneBuilder {
public:
	/**
	 * Add a primitive to primitives
	 * @param prim the primitive to be added
	 * @return a reference to this object so chain invoking can be performed
	 */
	SceneBuilder& addPrimitive(const std::shared_ptr<Primitive>& prim)
	{
		primitives.emplace_back(prim);
		return *this;
	}
	/**
	 * Add a light to lights
	 * @param light the light to be added
	 * @return a reference to this object so chain invoking can be performed
	 */
	SceneBuilder& addLight(const std::shared_ptr<Light>& light)
	{
		lights.emplace_back(light);
		if (light->isInfinite())
			infiniteLights.emplace_back(light);
		return *this;
	}
	/**
	 * Set primitives. Note that former content in primitives will be cleared
	 * @param primitives primitives to be set
	 * @return a reference to this object so chain invoking can be performed
	 */
	SceneBuilder& setPrimitives(const std::vector<std::shared_ptr<Primitive>>& primitives)
	{
		this->primitives = primitives;
		return *this;
	}
	/**
	 * Set lights. Note that former content in lights will be cleared
	 * @param lights lights to be set
	 * @return a reference to this object so chain invoking can be performed
	 */
	SceneBuilder& setLights(const std::vector<std::shared_ptr<Light>>& lights)
	{
		this->lights = lights;
		return *this;
	}
	/**
	 * Set environment light. This method assumes the environment light is already in lights. If you
	 * need to add environment light to lights. use addEnvironment
	 * @param env the environment light
	 * @return a reference to this object so chain invoking can be performed
	 */
	SceneBuilder& setEnvironment(const std::shared_ptr<Light>& env)
	{
		environment = env;
		return *this;
	}
	/**
	 * Set environment light respect to its index in lights. If you need to add environment light to
	 * lights. use addEnvironment
	 * @param index the index of environment light in lights
	 * @return a reference to this object so chain invoking can be performed
	 */
	SceneBuilder& setEnvironment(uint32_t index)
	{
		environment = lights[index];
		return *this;
	}
	SceneBuilder& addEnvironment(const std::shared_ptr<Light>& env)
	{
		environment = env;
		return addLight(env);
	}
	SceneBuilder& setHitSolverType(HitSolverType type)
	{
		hitSolverType = type;
		return *this;
	}
	std::shared_ptr<Scene> build() const;

protected:
	std::unique_ptr<HitSolver> buildHitSolver(const std::shared_ptr<Scene>& scene,
											  HitSolverType type) const;
	std::vector<std::shared_ptr<Primitive>> primitives;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Light>> infiniteLights;
	std::shared_ptr<Light> environment;
	HitSolverType hitSolverType = HitSolverType::UNKNOWN;
};

}  // namespace xd
#endif	// XD_RT_SCENEBUILDER_H

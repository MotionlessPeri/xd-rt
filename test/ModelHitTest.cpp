//
// Created by Frank on 2023/8/19.
//

#include "Model.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(ModelHitTestSuite, SphereHitTest)
{
	const Vector3f center{0, 0, 0};
	const double radius = 1.;
	auto sphere = std::make_shared<Sphere>(center, radius);

	const float sqrt3 = std::sqrtf(3.f);
	const float v0 = std::acosf(sqrt3 / 3.f) / PI;
	// intersect
	const Vector3f origin0{0, 0, 0};
	const Vector3f direction0 = Vector3f{1, 1, 1}.normalized();
	const Ray ray0{origin0, direction0};
	HitRecord hit0;
	auto b0 = sphere->hit(ray0, hit0);
	EXPECT_TRUE(b0);
	EXPECT_FLOAT_EQ(hit0.tHit, 1.);
	EXPECT_TRUE(hit0.uv.isApprox(Vector2f{v0, 1.f / 8.f}));

	// tangency but not intersect
	const Vector3f origin1{0, 0, 1};
	const Vector3f direction1 = Vector3f{1, 0, 0}.normalized();
	const Ray ray1{origin1, direction1};
	HitRecord hit1;
	auto b1 = sphere->hit(ray1, hit1);
	EXPECT_FALSE(b1);

	// tangency and intersect
	const Vector3f origin2{1, 1, 1};
	const Vector3f direction2 = Vector3f{-1, -1, 0}.normalized();
	const Ray ray2{origin2, direction2};
	HitRecord hit2;
	auto b2 = sphere->hit(ray2, hit2);
	EXPECT_TRUE(b2);
	EXPECT_FLOAT_EQ(hit2.tHit, std::sqrt(2));
	EXPECT_TRUE(hit2.uv.isApprox(Vector2f{0.f, 1.f / 8.f}));

	// not intersect at all
	const Vector3f origin3{0, 0, 2};
	const Vector3f direction3 = Vector3f{-1, -1, -1}.normalized();
	const Ray ray3{origin3, direction3};
	HitRecord hit3;
	auto b3 = sphere->hit(ray3, hit3);
	EXPECT_FALSE(b3);

	// intersect but in wrong direction
	const Vector3f origin4{0, 0, 2};
	const Vector3f direction4 = Vector3f{0, 0, 1}.normalized();
	const Ray ray4{origin4, direction4};
	HitRecord hit4;
	auto b4 = sphere->hit(ray4, hit4);
	EXPECT_FALSE(b4);
}

#include <oneapi/tbb.h>
#include "CameraFactory.h"
TEST(ModelHitTestSuite, SphereHitTest2)
{
	const float radius = 400;
	const Vector3f center{0, 0, 0};
	Sphere sphere{center, radius};
	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f pos{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(pos, center, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();
	auto sampler = std::make_shared<SimpleSampler>(width, height);
	const auto samples = sampler->generateSamples();

	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (sphere.hit(ray, rec)) {
								  film->addSample({rec.uv.x(), rec.uv.y(), 1}, sample);
							  }
						  }
					  });
	const std::string hdrPath = R"(D:\sphere_hit_uv.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}
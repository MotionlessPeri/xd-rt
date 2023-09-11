#include <oneapi/tbb.h>
#include <numeric>
#include "CameraFactory.h"
#include "MeshLoader.h"
#include "Triangle.h"
using namespace xd;
int main()
{
	ObjLoader loader;
	auto mesh = loader.load(R"(D:\qem-test.obj)");

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -2};
	const Vector3f origin{0, 0, 0};
	const float rightNorm = 3.f;
	const Vector3f right{rightNorm, 0, 0};
	const Vector3f up{0, rightNorm / width * height, 0};

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();

	const uint32_t row = 264u;
	const uint32_t col = 474u;
	const uint32_t sampleIdx = row * width + col;
	const auto& sample = samples[sampleIdx];
	auto ray = cam->generateRay(sample);
	HitRecord rec;
	if (mesh->hit(ray, rec)) {
		const float floatIdx = (float)rec.debug + 1.f;
		film->addSample({floatIdx, floatIdx, floatIdx}, sample);
	}

	const std::string hdrPath = R"(D:\obj_load_and_hit_debug_idx.hdr)";
}
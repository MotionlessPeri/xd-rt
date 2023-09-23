//
// Created by Frank on 2023/8/28.
//
#include <oneapi/tbb.h>
#include <atomic>
#include <numeric>

#include "Distribution.h"
#include "Film.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(DistribTestSuite, UniformHemisphereTest)
{
	auto dis = std::make_shared<UniformHemisphere>();
	auto film =
		std::make_shared<Film>(Vector3f{0, 0, 0}, Vector3f{1, 0, 0}, Vector3f{0, 1, 0}, 100, 100);
	constexpr auto nSamples = 10000u;
	for (auto i = 0u; i < nSamples; ++i) {
		const auto sample = dis->operator()();
		const auto x = (sample.x() + 1.f) / 2.f;
		const auto y = (sample.y() + 1.f) / 2.f;
		film->addSample({1, 1, 1}, {x, y});
	}
	EXPECT_NO_THROW(film->saveToFile(R"(D:\uniform_hemisphere_test.hdr)"));
}

TEST(DistribTestSuite, PieceWise1DCtorTest)
{
	std::vector<float> pdf{0.2, 0.6, 0.1, 0.1};
	PieceWise1D dis(pdf);
	const auto& cdf = dis.getCdfs();
	EXPECT_FLOAT_EQ(cdf[0], pdf[0]);
	EXPECT_FLOAT_EQ(cdf[1], 0.8);
	EXPECT_FLOAT_EQ(cdf[2], 0.9);
	EXPECT_FLOAT_EQ(cdf[3], 1.0);

	float arr[] = {0.4, 0.3, 0.2, 0.1};
	PieceWise1D dis2{arr, 4};
	const auto& cdf2 = dis2.getCdfs();
	EXPECT_FLOAT_EQ(cdf2[0], 0.4);
	EXPECT_FLOAT_EQ(cdf2[1], 0.7);
	EXPECT_FLOAT_EQ(cdf2[2], 0.9);
	EXPECT_FLOAT_EQ(cdf2[3], 1.0);

	std::vector<float> weights{1, 2, 3, 4};
	PieceWise1D dis3{weights};
	const auto& cdf3 = dis3.getCdfs();
	EXPECT_FLOAT_EQ(cdf3[0], 0.1);
	EXPECT_FLOAT_EQ(cdf3[1], 0.3);
	EXPECT_FLOAT_EQ(cdf3[2], 0.6);
	EXPECT_FLOAT_EQ(cdf3[3], 1.0);

	uint32_t offset;
	auto sample = dis3(offset);
}

TEST(DistribTestSuite, PieceWise1DGenerateTest)
{
	std::vector<float> pdf{0.2, 0.6, 0.1, 0.1};
	PieceWise1D dis(pdf);
	std::vector<uint32_t> counts(pdf.size(), 0u);
	constexpr uint32_t TOTAL = 100000u;
	const auto& cdf = dis.getCdfs();
	const float delta = 1.f / pdf.size();
	for (auto i = 0u; i < TOTAL; ++i) {
		uint32_t offset;
		float p1;
		const auto val = dis(p1, offset);
		const auto p2 = dis.getPdf(val);
		EXPECT_FLOAT_EQ(p1, p2);
		for (int n = 0u; n < pdf.size(); ++n) {
			if (val < (n + 1) * delta) {
				counts[n]++;
				break;
			}
		}
	}
	const auto total = std::accumulate(counts.begin(), counts.end(), 0u);
	constexpr float eps = 1e-2;
	for (auto i = 0u; i < pdf.size(); ++i) {
		EXPECT_TRUE(std::fabs((float)counts[i] / TOTAL - pdf[i]) < eps);
	}
}

TEST(DistribTestSuite, PieceWise2DCtorTest)
{
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	constexpr float du = 1.f / width;
	constexpr float dv = 1.f / height;
	std::vector<float> weights{2, 1, 2,	  // NOLINT
							   4, 5, 6};  // NOLINT
	PieceWise2D dis{weights, 3, 2};
	const auto sum = std::accumulate(weights.cbegin(), weights.cend(), 0);
	std::vector<float> ps(weights);
	for (auto& val : ps)
		val /= sum;
	for (auto row = 0u; row < height; ++row) {
		for (auto col = 0u; col < width; ++col) {
			const auto i = row * width + col;
			const float u = (col * du + du / 2.f);
			const float v = (row * dv + dv / 2.f);
			EXPECT_FLOAT_EQ(ps[i], dis.getPdf({u, v}));
		}
	}
}

TEST(DistribTestSuite, PieceWise2DGenTest)
{
	std::vector<float> weights{2, 1, 2,	  // NOLINT
							   4, 5, 6};  // NOLINT
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	constexpr float du = 1.f / width;
	constexpr float dv = 1.f / height;
	PieceWise2D dis{weights, width, height};

	constexpr uint32_t COUNT = 10000000u;
	std::vector<uint32_t> counts(weights.size(), 0u);
	for (auto i = 0u; i < COUNT; ++i) {
		float pdf;
		const auto sample = dis(pdf);
		const auto pdf2 = dis.getPdf(sample);
		// EXPECT_FLOAT_EQ(pdf, pdf2);
		const float dv = 1.f / height;
		const float du = 1.f / width;
		const uint32_t vIdx = std::clamp((uint32_t)std::floorf(sample.y() / dv), 0u, height - 1);
		const uint32_t uIdx = std::clamp((uint32_t)std::floorf(sample.x() / du), 0u, width - 1);
		counts[vIdx * width + uIdx]++;
	}
	constexpr float eps = 1e-2;
	for (auto row = 0u; row < height; ++row) {
		for (auto col = 0u; col < width; ++col) {
			const auto i = row * width + col;
			const float u = (col * du + du / 2.f);
			const float v = (row * dv + dv / 2.f);
			const auto prob = counts[i] * 1.f / COUNT;
			EXPECT_TRUE(std::fabs(prob - dis.getPdf({u, v})) < eps);
		}
	}
}

#include "TextureFactory.h"
TEST(DistribTestSuite, PieceWise2DGenTest2)
{
	auto texture = TextureFactory::loadSphereTexture3f(R"(D:\dome.hdr)");
	const auto width = texture->getWidth();
	const auto height = texture->getHeight();
	const auto& data = texture->getImage();
	const auto pixelCnt = width * height;
	std::vector<float> weights(pixelCnt, 0.f);
	for (auto i = 0u; i < pixelCnt; ++i) {
		weights[i] = rgbToLuminance(data[i]);
	}
	constexpr uint32_t COUNT = 100000u;
	struct Count {
		int row;
		int col;
		int cnt = 0;
		bool operator<(const Count& rhs) const { return cnt < rhs.cnt; }
		bool operator>(const Count& rhs) const { return rhs < *this; }
		bool operator<=(const Count& rhs) const { return !(rhs < *this); }
		bool operator>=(const Count& rhs) const { return !(*this < rhs); }
	};
	std::vector<Count> counts(weights.size());
	for (auto row = 0u; row < height; ++row) {
		for (auto col = 0u; col < width; ++col) {
			const auto index = row * width + col;
			counts[row * width + col].row = row;
			counts[row * width + col].col = col;
		}
	}
	PieceWise2D dis{weights, width, height};
	for (auto i = 0u; i < COUNT; ++i) {
		float pdf;
		const auto sample = dis(pdf);
		const float dv = 1.f / height;
		const float du = 1.f / width;
		const uint32_t vIdx = std::clamp((uint32_t)std::floorf(sample.y() / dv), 0u, height - 1);
		const uint32_t uIdx = std::clamp((uint32_t)std::floorf(sample.x() / du), 0u, width - 1);
		counts[vIdx * width + uIdx].cnt++;
	}
	std::sort(counts.begin(), counts.end(), std::greater<Count>{});
	__debugbreak();
}

TEST(DistribTestSuite, CosineHemisphereTest)
{
	auto dis = std::make_shared<CosineHemisphere>();
	auto film =
		std::make_shared<Film>(Vector3f{0, 0, 0}, Vector3f{1, 0, 0}, Vector3f{0, 1, 0}, 100, 100);
	constexpr auto nSamples = 10000u;
	for (auto i = 0u; i < nSamples; ++i) {
		const auto sample = dis->operator()();
		const auto x = (sample.x() + 1.f) / 2.f;
		const auto y = (sample.y() + 1.f) / 2.f;
		film->addSample({1, 1, 1}, {x, y});
	}
	EXPECT_NO_THROW(film->saveToFile(R"(D:\cosine_hemisphere_test.hdr)"));
}
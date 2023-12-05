//
// Created by Frank on 2023/8/16.
//
#include "../src/core/Film.h"
#include "gtest/gtest.h"
using namespace xd;

TEST(FilmTestSuite, sampleTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;

	const Vector2f sample0{0.5, 0.5};  // should be index 0
	const ColorRGB color0{0.2, 0.3, 0.6};
	const Vector2f sample1{2.5, 1.5};  // should be index 5
	const ColorRGB color1{0.3, 0.7, 0.2};
	const ColorRGB zero{0, 0, 0};

	const Vector2f sample2{0, 0};	   // should be index 0
	const Vector2f sample3{1.5, 1.5};  // should be index 4
	Film film{center, right, up, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});
	tile->addSample(color0, sample0);
	tile->addSample(color1, sample1);
	film.mergeTileToFilm(std::move(tile));

	const auto res0 = film.getPixelColor(sample0);
	const auto res1 = film.getPixelColor(sample1);
	const auto res2 = film.getPixelColor(sample2);
	const auto res3 = film.getPixelColor(sample3);

	EXPECT_TRUE(res0.isApprox(color0));
	EXPECT_TRUE(res1.isApprox(color1));
	EXPECT_TRUE(res2.isApprox(color0));
	EXPECT_TRUE(res3.isApprox(zero));
}

#include <condition_variable>
#include <random>
TEST(FilmTestSuite, ParallelMergeToFileTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	Film film{center, right, up, width, height};

	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_real_distribution<float> uniform_dist{};

	std::array<Vector3f, 6> colors;
	for (auto& color : colors) {
		color = {uniform_dist(e1), uniform_dist(e1), uniform_dist(e1)};
	}

	std::array<std::unique_ptr<FilmTile>, 6> tiles;
	for (auto i = 0u; i < 6; ++i) {
		const auto y = i / width;
		const auto x = i % width;
		const Vector2i topLeft{x, y};
		tiles[i] = film.getTile(topLeft, topLeft);
	}

	std::array<std::thread, 6> threads;

	class Semaphore {
	public:
		Semaphore(int count_ = 0) : count(count_) {}

		inline void notify()
		{
			std::unique_lock<std::mutex> lock(mtx);
			count++;
			cv.notify_one();
		}

		inline void wait()
		{
			std::unique_lock<std::mutex> lock(mtx);

			while (count < 0) {
				cv.wait(lock);
			}
			count--;
		}

	private:
		std::mutex mtx;
		std::condition_variable cv;
		int count;
	};

	Semaphore semaphore{-6};
	const auto func = [&](std::unique_ptr<FilmTile> tile, const ColorRGB& color) {
		Vector2f center{tile->topLeft.x() + 0.5f, tile->topLeft.y() + 0.5f};
		tile->addSample(color, center);
		film.mergeTileToFilm(std::move(tile));
		semaphore.notify();
	};
	for (auto i = 0u; i < 6; ++i) {
		threads[i] = std::thread{func, std::move(tiles[i]), colors[i]};
		threads[i].detach();
	}
	semaphore.wait();
	for (auto i = 0u; i < 6; ++i) {
		EXPECT_TRUE(film.getPixelColor(i).isApprox(colors[i]));
	}
}

TEST(FilmTestSuite, saveToFileFailedTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	Film film{center, right, up, width, height};
	const std::string fileName = R"(D:\wtf.cpp)";
	EXPECT_THROW({ film.saveToFile(fileName, {}); }, std::runtime_error);
}

TEST(FilmTestSuite, saveToFileTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	Film film{center, right, up, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});

	constexpr double oneSixth = 1. / 6.;
	constexpr double oneQuater = 1. / 4.;
	const Vector2f sample0{0.5, 0.5};  // should be index 0
	const ColorRGB red{0.8, 0.1, 0.1};
	const Vector2f sample4{1.5, 1.5};  // should be index 4
	const ColorRGB green{0.1, 0.7, 0.1};
	const Vector2f sample5{2.5, 1.5};  // should be index 5
	const ColorRGB white{1, 1, 1};

	tile->addSample(red, sample0);
	tile->addSample(green, sample4);
	tile->addSample(white, sample5);

	film.mergeTileToFilm(std::move(tile));
	const std::string fileName = R"(D:\saveToFileTest.hdr)";
	EXPECT_NO_THROW(film.saveToFile(fileName, {}););
}

TEST(FilmTestSuite, FilmTileIndexTest)
{
	FilmTile tile{{16, 16}, {31, 31}};
	EXPECT_EQ(tile.getIndexFromSample({16.2, 16.3}), 0);
	EXPECT_EQ(tile.getIndexFromSample({18.1, 16.3}), 2);
	EXPECT_EQ(tile.getIndexFromSample({16.1, 17.3}), 16);
	EXPECT_EQ(tile.getIndexFromSample({31.5, 31.5}), 255);
	EXPECT_EQ(tile.getIndexFromSample({14.5, 31.5}), -1);
	EXPECT_EQ(tile.getIndexFromSample({14.5, 10.5}), -1);

	EXPECT_EQ(tile.getSampleCoordFromIndex(0), Vector2i(16, 16));
	EXPECT_EQ(tile.getSampleCoordFromIndex(5), Vector2i(21, 16));
	EXPECT_EQ(tile.getSampleCoordFromIndex(16), Vector2i(16, 17));
	EXPECT_EQ(tile.getSampleCoordFromIndex(255), Vector2i(31, 31));
}

#include <ranges>
TEST(FilmTestSuite, FilmWeightTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 2;
	constexpr uint32_t height = 1;
	Film film{center, right, up, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});

	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_real_distribution<float> uniform_dist{};
	const Vector3f color0{uniform_dist(e1), uniform_dist(e1), uniform_dist(e1)};

	Vector3f color1{0, 0, 0};
	float weight1 = 0.f;
	for (auto i : std::views::iota(0u, 100u)) {
		tile->addSample(color0, {0.5, 0.5});

		const Vector3f tmpColor{uniform_dist(e1), uniform_dist(e1), uniform_dist(e1)};
		const float tmpWeight = uniform_dist(e1);
		color1 += tmpColor * tmpWeight;
		weight1 += tmpWeight;
		tile->addSample(tmpColor, {1.5, 0.5}, tmpWeight);
	}
	color1 /= weight1;

	film.mergeTileToFilm(std::move(tile));

	EXPECT_TRUE(color0.isApprox(film.getPixelColor({0.5, 0.5})));
	EXPECT_TRUE(color1.isApprox(film.getPixelColor({1.5, 0.5})));
}

TEST(FilmTestSuite, RangeForTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 3;
	Film film{center, right, up, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});

	const std::array<Vector2i, 9> expects{
		Vector2i{0, 0}, Vector2i{1, 0}, Vector2i{2, 0}, Vector2i{0, 1}, Vector2i{1, 1},
		Vector2i{2, 1}, Vector2i{0, 2}, Vector2i{1, 2}, Vector2i{2, 2},
	};
	int i = 0;

	EXPECT_EQ(*tile->begin(), Vector2i(0, 0));
	EXPECT_EQ(*tile->end(), Vector2i(0, 3));

	for (const auto& pixel : *tile) {
		std::cout << pixel.transpose() << std::endl;
		EXPECT_EQ(pixel, expects[i++]);
	}
}

TEST(FilmTestSuite, SampleToWorldTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{1.5, 0, 0};
	const Vector3f up{0, 1.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 3;
	Film film{center, right, up, width, height};
	EXPECT_TRUE(film.getWorldCoordsFromSample({0.9, 0.6}).isApprox(Vector3f{-0.6, 0.9, 0}));
	EXPECT_TRUE(film.getWorldCoordsFromSample({1.5, 1.5}).isApprox(Vector3f{0, 0, 0}));
}
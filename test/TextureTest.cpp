//
// Created by Frank on 2023/9/2.
//
#include "gtest/gtest.h"

#include "Sampler.h"
#include "Texture.h"
using namespace xd;
TEST(TextureTestSuite, ConstantTextureSampleTest)
{
	const float fParam = 0.6f;
	const Vector3f cParam{0.1, 0.7, 0.2};
	const Vector2f samplePos{0.4, 0.6};
	auto textureF = std::make_shared<ConstantTextureF>(fParam);
	auto textureC = std::make_shared<ConstantTextureColor>(cParam);
	EXPECT_EQ(textureF->sample(samplePos), fParam);
	EXPECT_TRUE(cParam.isApprox(textureC->sample(samplePos)));
}
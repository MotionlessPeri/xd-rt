//
// Created by Frank on 2023/12/19.
//
#include "MathUtil.h"
#include "bxdf/Fresnel.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(PerfectFresnelTestSuite, FresnelFunctionTest)
{
	// Note: Data from the matlab code in
	// https://webs.optics.arizona.edu/gsmith/FresnelCalculator.html
	{
		const auto etaI = 1.f;
		const auto etaT = 1.f;
		const auto eta = etaT / etaI;
		const auto thetaI = toRadians(45.f);
		const auto cosThetaI = std::cos(thetaI);
		const auto sin2ThetaI = 1 - cosThetaI * cosThetaI;
		const auto sin2ThetaT = sin2ThetaI / (eta * eta);
		const auto cosThetaT = std::sqrt(1 - sin2ThetaT);
		const auto f = FresnelReal::fresnel(eta, cosThetaI, cosThetaT);
		EXPECT_TRUE(fuzzyEqual(f, 0.f));
	}

	{
		const auto etaI = 1.f;
		const auto etaT = 1.33f;
		const auto eta = etaT / etaI;
		const auto thetaI = toRadians(45.f);
		const auto cosThetaI = std::cos(thetaI);
		const auto sin2ThetaI = 1 - cosThetaI * cosThetaI;
		const auto sin2ThetaT = sin2ThetaI / (eta * eta);
		const auto cosThetaT = std::sqrt(1 - sin2ThetaT);
		const auto f = FresnelReal::fresnel(eta, cosThetaI, cosThetaT);
		EXPECT_TRUE(fuzzyEqual(f, 0.027521f));
	}

	{
		const auto etaI = 2.5f;
		const auto etaT = 1.5f;
		const auto eta = etaT / etaI;
		const auto thetaI = toRadians(30.f);
		const auto cosThetaI = std::cos(thetaI);
		const auto sin2ThetaI = 1 - cosThetaI * cosThetaI;
		const auto sin2ThetaT = sin2ThetaI / (eta * eta);
		const auto cosThetaT = std::sqrt(1 - sin2ThetaT);
		const auto f = FresnelReal::fresnel(eta, cosThetaI, cosThetaT);
		EXPECT_TRUE(fuzzyEqual(f, 0.1f));
	}

	{
		const auto etaI = 1.5f;
		const std::complex<float> etaT{0.56f, 11.21f};	// gold 1610nm
		const auto eta = etaT / etaI;
		const auto thetaI = toRadians(82.5f);
		const auto cosThetaI = std::cosf(thetaI);
		const auto sin2ThetaI = 1 - cosThetaI * cosThetaI;
		const auto sin2ThetaT = sin2ThetaI / (eta * eta);
		const auto cosThetaT = std::sqrt(1.f - sin2ThetaT);
		const auto f = FresnelComplex::fresnel(eta, cosThetaI, cosThetaT);
		EXPECT_TRUE(fuzzyEqual(f, 0.95003f));
	}

	{
		const auto etaI = 2.5f;
		const std::complex<float> etaT{1.28f, 2.207f};	// copper 413nm
		const auto eta = etaT / etaI;
		const auto thetaI = toRadians(30.f);
		const auto cosThetaI = std::cosf(thetaI);
		const auto sin2ThetaI = 1 - cosThetaI * cosThetaI;
		const auto sin2ThetaT = sin2ThetaI / (eta * eta);
		const auto cosThetaT = std::sqrt(1.f - sin2ThetaT);
		const auto f = FresnelComplex::fresnel(eta, cosThetaI, cosThetaT);
		EXPECT_TRUE(fuzzyEqual(f, 0.346938f));
	}
}
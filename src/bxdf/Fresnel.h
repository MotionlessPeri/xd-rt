//
// Created by Frank on 2023/12/18.
//

#ifndef XD_RT_FRESNEL_H
#define XD_RT_FRESNEL_H
#include <complex>
namespace xd {
// Note: Because static keyword cannot be used to decorate explicit specialization function, we use
// the anonymous namespace for internal linkage
namespace {
template <typename T>
inline float customNorm(T val)
{
	return val * val;
}

// Specialization for std::complex<float>
template <>
inline float customNorm(std::complex<float> val)
{
	return std::norm(val);
}
}  // namespace

template <typename EtaType>
struct FresnelTemp {
	/**
	 * calculate the reflected percentage of light of a given medium boundary and incident angle.
	 * Note that the function requires both cosThetaI and cosThetaT, meaning that internal
	 * reflection must be handled by user.
	 * @param eta The quotient between ior of transmitted medium and incident medium. Note that the
	 * incident ior must be a real value because light can not travel in conductors
	 * @param cosThetaI cosine of incident azimuth angle. cosThetaI will always be a real value.
	 * @param cosThetaT cosine of transmitted azimuth angle.
	 * @return the reflected percentage of light.
	 */
	static float fresnel(EtaType eta, float cosThetaI, EtaType cosThetaT)
	{
		// Note: we can use static operator() if c++23 is available
		const auto etaCosThetaI = eta * cosThetaI;
		const auto rParallel = (etaCosThetaI - cosThetaT) / (etaCosThetaI + cosThetaT);
		const auto etaCosThetaT = eta * cosThetaT;
		const auto rPerpendicular = (cosThetaI - etaCosThetaT) / (cosThetaI + etaCosThetaT);
		const auto fresnel = 0.5f * (customNorm(rParallel) + customNorm(rPerpendicular));
		return fresnel;
	}
};

using FresnelReal = FresnelTemp<float>;
using FresnelComplex = FresnelTemp<std::complex<float>>;
}  // namespace xd
#endif	// XD_RT_FRESNEL_H

//
// Created by Frank on 2023/9/30.
//

#ifndef XD_RT_EMBREEGLOBAL_H
#define XD_RT_EMBREEGLOBAL_H
#include "embree4/rtcore.h"
namespace xd {
class EmbreeGlobal {
public:
	static EmbreeGlobal& get()
	{
		static EmbreeGlobal* singleton = new EmbreeGlobal;
		return *singleton;
	}
	RTCDevice device;

protected:
	EmbreeGlobal();
	~EmbreeGlobal();
};
}  // namespace xd
#endif	// XD_RT_EMBREEGLOBAL_H

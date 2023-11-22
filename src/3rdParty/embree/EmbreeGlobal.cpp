//
// Created by Frank on 2023/9/30.
//
#include "EmbreeGlobal.h"

#include <iostream>
using namespace xd;
EmbreeGlobal::EmbreeGlobal()
{
	device = rtcNewDevice(nullptr);
	rtcSetDeviceErrorFunction(
		device,
		[](void* userPtr, RTCError code, const char* str) -> void {
			std::cout << str << std::endl;
		},
		nullptr);
}

EmbreeGlobal::~EmbreeGlobal()
{
	rtcReleaseDevice(device);
}
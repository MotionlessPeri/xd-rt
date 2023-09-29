//
// Created by Frank on 2023/9/30.
//
#include "EmbreeGeomManager.h"
#include "EmbreeGlobal.h"
#include "Triangle.h"
using namespace xd;
EmbreeGeomManager::EmbreeGeomManager()
{
	device = EmbreeGlobal::get().device;
}
#include <iostream>
RTCGeometry EmbreeGeomManager::getOrCreateGeom(const Model* model)
{
	const auto queryIt = geoms.find(model);
	if (queryIt != geoms.end()) {
		return queryIt->second;
	}
	const auto* mesh = dynamic_cast<const TriangleMesh*>(model);
	if (mesh == nullptr)
		return nullptr;
	auto geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

	const auto& positions = mesh->getPositions();
	RTCBuffer vBuffer =
		rtcNewSharedBuffer(device, (void*)positions.data(), positions.size() * sizeof(float));

	const auto& indices = mesh->getIndices();
	RTCBuffer iBuffer =
		rtcNewSharedBuffer(device, (void*)indices.data(), indices.size() * sizeof(uint32_t));

	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vBuffer, 0,
						 3 * sizeof(float), positions.size() / 3);
	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, iBuffer, 0,
						 3 * sizeof(uint32_t), indices.size() / 3);
	auto err = rtcGetDeviceError(device);
	unsigned int attribCnt = 0u;
	const auto& normals = mesh->getNormals();
	const auto& uvs = mesh->getUVs();
	if (!uvs.empty()) {
		RTCBuffer buffer =
			rtcNewSharedBuffer(device, (void*)uvs.data(), uvs.size() * sizeof(float));
		attribCnt++;
		rtcSetGeometryVertexAttributeCount(geom, attribCnt);
		rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
							 (unsigned int)VertexAttributeSlot::UV, RTC_FORMAT_FLOAT2, buffer, 0,
							 2 * sizeof(float), uvs.size() / 2);
	}
	if (!normals.empty()) {
		RTCBuffer buffer =
			rtcNewSharedBuffer(device, (void*)normals.data(), normals.size() * sizeof(float));
		attribCnt++;
		rtcSetGeometryVertexAttributeCount(geom, attribCnt);
		rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, (unsigned int)VertexAttributeSlot::NORMAL, RTC_FORMAT_FLOAT3, buffer,
							 0,
							 3 * sizeof(float), normals.size() / 3);
	}

	const auto& tangents = mesh->getTangents();
	if (!tangents.empty()) {
		RTCBuffer buffer =
			rtcNewSharedBuffer(device, (void*)tangents.data(), tangents.size() * sizeof(float));
		attribCnt++;
		rtcSetGeometryVertexAttributeCount(geom, attribCnt);
		rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
							 (unsigned int)VertexAttributeSlot::TANGENT, RTC_FORMAT_FLOAT3, buffer,
							 0,
							 3 * sizeof(float), tangents.size() / 3);
	}


	// TODO: we can either give the interpolate work to embree or handle it ourselves
	// If we decided embree to take care of that, more vertex attributes must be set to geom
	rtcCommitGeometry(geom);
	err = rtcGetDeviceError(device);
	geoms[model] = geom;
	return geom;
}

void EmbreeGeomManager::releaseGeom(const Model* model)
{
	auto queryIt = geoms.find(model);
	if (queryIt != geoms.end()) {
		rtcReleaseGeometry(queryIt->second);
		geoms.erase(queryIt);
	}
}

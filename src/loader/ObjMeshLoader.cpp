//
// Created by Frank on 2023/9/8.
//

#include "ObjMeshLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "model/Triangle.h"
using namespace xd;
static Vector2f parseVector2(const std::string& aString)
{
	Vector2f result;
	std::istringstream sstr(aString);
	sstr >> result.x();
	sstr >> result.y();
	return result;
}

static Vector3f parseVector3(const std::string& aString)
{
	Vector3f result;
	std::istringstream sstr(aString);
	sstr >> result.x();
	sstr >> result.y();
	sstr >> result.z();
	return result;
}

constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();
struct ObjFaceIndex {
	uint32_t i = invalidIndex;
	uint32_t it = invalidIndex;
	uint32_t in = invalidIndex;
};
static std::array<ObjFaceIndex, 3> parseFace(const std::string& aStr)
{
	// Start by finding the first and last non-whitespace char (trim)
	size_t l = aStr.size();
	size_t pos = 0, strEnd = l - 1;
	while ((pos < strEnd) && ((aStr[pos] == ' ') || (aStr[pos] == '\t')))
		++pos;
	while ((strEnd > pos) && ((aStr[strEnd] == ' ') || (aStr[strEnd] == '\t')))
		--strEnd;

	std::array<ObjFaceIndex, 3> res{};
	uint32_t i = 0;
	// Extract three face corners (one triangle)
	while ((pos <= strEnd) && (aStr[pos] != ' ') && (aStr[pos] != '\t')) {
		// Extract three /-separated strings (v/vt/vn)
		if (i >= 3) {
			throw std::runtime_error("Only triangle obj file is supported for now.");
		}
		std::string v_s[3];
		int j = 0;
		while ((pos <= strEnd) && (aStr[pos] != ' ') && (aStr[pos] != '\t') && (j < 3)) {
			if (aStr[pos] != '/')
				v_s[j] += aStr[pos];
			else
				++j;
			++pos;
		}

		// Skip whitespaces
		while ((pos <= strEnd) && ((aStr[pos] == ' ') || (aStr[pos] == '\t')))
			++pos;

		// Convert the strings to integers
		const auto assignWithException = [](const std::string& strVal, uint32_t& place) {
			uint32_t value = 0;
			if (!strVal.empty()) {
				std::istringstream ss(strVal);
				ss >> value;
				if (value > 0)
					value--;
				else if (value < 0)
					throw std::runtime_error(
						"Negative vertex references in OBJ files are not supported.");
				else
					throw std::runtime_error("Invalid index (zero) in OBJ file.");
				place = value;
			}
		};
		assignWithException(v_s[0], res[i].i);
		assignWithException(v_s[1], res[i].it);
		assignWithException(v_s[2], res[i].in);
		++i;
	}
	return res;
}
std::shared_ptr<TriangleMesh> ObjMeshLoader::load(const std::string& path,
												  const LoadMeshOptions& options)
{
	// NOTE: the code is copied from openctm viewer's code
	// Open the input file
	std::ifstream inFile(path, std::ios_base::in);
	if (inFile.fail())
		throw std::runtime_error("Could not open input file.");

	std::vector<float> positions;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<std::string> faceStrs;
	// Parse the file
	while (!inFile.eof()) {
		// Read one line from the file (concatenate lines that end with "\")
		std::string line;
		std::getline(inFile, line);
		while ((line.size() > 0) && (line[line.size() - 1] == '\\') && !inFile.eof()) {
			std::string nextLine;
			getline(inFile, nextLine);
			line = line.substr(0, line.size() - 1) + std::string(" ") + nextLine;
		}

		// Parse the line, if it is non-empty
		if (line.size() >= 1) {
			if (line.substr(0, 2) == "v ") {
				const auto vec = parseVector3(line.substr(2));
				positions.push_back(vec(0));
				positions.push_back(vec(1));
				positions.push_back(vec(2));
			}
			else if (line.substr(0, 3) == "vt ") {
				const auto vec = parseVector2(line.substr(3));
				uvs.push_back(vec(0));
				uvs.push_back(vec(1));
			}

			else if (line.substr(0, 3) == "vn ") {
				const auto vec = parseVector3(line.substr(3));
				normals.push_back(vec(0));
				normals.push_back(vec(1));
				normals.push_back(vec(2));
			}
			else if (line.substr(0, 2) == "f ")
				faceStrs.push_back(line.substr(2));
		}
	}

	const auto vertexCnt = faceStrs.size() * 3;
	const auto firstFace = parseFace(faceStrs.front());
	bool hasUV = firstFace[0].it != invalidIndex;
	bool hasNormal = firstFace[0].in != invalidIndex;
	std::vector<float> meshPos(vertexCnt * 3);

	std::vector<float> meshUV{};
	if (hasUV)
		meshUV.resize(vertexCnt * 2);

	std::vector<float> meshNormal{};
	if (hasNormal)
		meshNormal.resize(vertexCnt * 3);

	std::vector<float> meshTangent{};
	std::vector<uint32_t> meshIndices{};
	meshIndices.reserve(vertexCnt);
	uint32_t vIndex = 0;
	// Note: should we preserve topology(reuse vertices) or totally break it?
	for (const auto& faceStr : faceStrs) {
		const auto indices = parseFace(faceStr);
		for (int i = 0; i <= 2; ++i) {
			for (auto j = 0u; j < 3; ++j) {
				meshPos[3 * vIndex + j] = positions[3 * indices[i].i + j];
			}
			if (hasUV) {
				for (auto j = 0u; j < 2; ++j) {
					meshUV[2 * vIndex + j] = uvs[2 * indices[i].it + j];
				}
			}
			if (hasNormal) {
				for (auto j = 0u; j < 3; ++j) {
					meshNormal[3 * vIndex + j] = normals[3 * indices[i].in + j];
				}
			}
			meshIndices.emplace_back(vIndex);
			++vIndex;
		}
	}
	auto res = std::make_shared<TriangleMesh>(meshPos, meshUV, meshNormal, meshTangent, meshIndices,
											  options.method);
	return res;
}

#include"JObjFileLoader.h"
#include"../../DirectXEx/JDirectXCollisionEx.h"
#include"../../../Object/Resource/Mesh/JMeshStruct.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Resource/JResourcePathData.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Object/Resource/Model/JModelAttribute.h"

#include<fstream>
#include<io.h>

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		JObjFileLoaderImpl::FaceInfo::FaceInfo(const std::vector<XMINT3>& ptn)
			:ptn(ptn)
		{
			hasUV = true;
			hasNormal = true;
			const uint ptnCount = (uint)ptn.size();

			for (uint i = 0; i < ptnCount; ++i)
			{
				hasUV = hasUV && (ptn[i].y != -1);
				hasNormal = hasNormal && (ptn[i].z != -1);
			}
		}

		bool JObjFileLoaderImpl::LoadObjFile(const JResourcePathData& pathData, std::vector<JObjMeshPartData>& objMeshData, std::vector<JObjMatData>& objMatData, JModelAttribute& modelAttribute)
		{
			std::wstring next;
			std::wstring materialLibName;
			std::vector<std::string> meshName;
			std::vector<std::string> materialName;
			std::vector<XMFLOAT3> position;
			std::vector<XMFLOAT2> texture;
			std::vector<XMFLOAT3> normal;
			std::vector<uint> index;
			std::vector<uint>vertexCount;
			std::vector<uint>indexCount;
			std::vector<std::vector<FaceInfo>> faceInfo;

			float x, y, z;
			int meshCount = -1;
			std::wifstream stream;

			stream.open(pathData.wstrPath, std::ios::in, std::ios::binary);
			if (!stream.is_open())
			{
				MessageBox(0, L"Stream Fail", 0, 0);
				stream.close();
				return false;
			}

			while (stream >> next)
			{
				if (next == L"o")
				{
					std::getline(stream, next);
					meshName.push_back(JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next)));
					materialName.emplace_back(std::string());
					vertexCount.push_back(0);
					indexCount.push_back(0);
					faceInfo.emplace_back(std::vector<FaceInfo>());
					++meshCount;
				}
				else if (next == L"usemtl")
				{
					std::getline(stream, next);
					materialName[meshCount] = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
				}
				else if (next == L"mtllib")
				{
					std::getline(stream, next);
					materialLibName = JCommonUtility::EraseEmptySpace(next);
				}
				else if (next == L"v")
				{
					stream >> x >> y >> z;
					position.emplace_back(x, y, z);
					++vertexCount[meshCount];
				}
				else if (next == L"vt")
				{
					stream >> x >> y;
					texture.emplace_back(x, y);
				}
				else if (next == L"vn")
				{
					stream >> x >> y >> z;
					normal.emplace_back(x, y, z);
				}
				else if (next == L"f")
				{
					std::getline(stream, next);
					next = JCommonUtility::EraseEmptySpace(next);

					std::vector<XMINT3> ptn;
					std::wstring ptnIndexWstr;
					const uint nextCount = (uint)next.size();
					for (uint i = 0; i < nextCount; ++i)
					{
						if (next[i] != ' ')
							ptnIndexWstr.push_back(next[i]);
						else
						{
							int posIndex;
							int uvIndex;
							int normalIndex;
							GetVectorIndex(ptnIndexWstr, posIndex, uvIndex, normalIndex);
							ptn.emplace_back(posIndex, uvIndex, normalIndex);
							index.push_back(posIndex);
							ptnIndexWstr.clear();
						}
					}
					faceInfo[meshCount].emplace_back(FaceInfo(ptn));
					indexCount[meshCount] += nextCount;
				}
			}
			stream.close();

			bool hasMat = false;
			if (!materialLibName.empty())
				hasMat = LoadMatFile(materialLibName, objMatData);

			std::vector<JStaticMeshVertex> vertex(position.size());
			for (int i = 0; i < meshCount; ++i)
			{
				objMeshData.push_back(JObjMeshPartData());
				objMeshData[i].meshName = meshName[i];
				objMeshData[i].materialName = materialName[i];
				objMeshData[i].staticMeshData.indices32.resize(indexCount[i]);

				uint indexSum = 0;
				const int ptnIndexInfoCount = (int)faceInfo[i].size();
				for (int j = 0; j < ptnIndexInfoCount; ++j)
				{
					uint fCount = (uint)faceInfo[i][j].ptn.size();
					for (uint k = 0; k < fCount; ++k)
					{
						const uint pIndex = faceInfo[i][j].ptn[k].x;
						const uint nIndex = faceInfo[i][j].ptn[k].y;
						const uint tIndex = faceInfo[i][j].ptn[k].z;
						vertex[pIndex] = JStaticMeshVertex(position[pIndex], normal[nIndex], texture[tIndex]);
						objMeshData[i].staticMeshData.indices32[indexSum] = pIndex - indexCount[i];
						++indexSum;
					}
				}
			}

			XMFLOAT3 vMinf3 = JMathHelper::Min3F();
			XMFLOAT3 vMaxf3 = JMathHelper::Max3F();

			bool hasTangent = true;
			bool isSameFace = true;

			XMVECTOR modelMin = XMLoadFloat3(&vMinf3);
			XMVECTOR modelMax = XMLoadFloat3(&vMaxf3);

			for (int i = 0; i < meshCount; ++i)
			{
				XMVECTOR meshMin = XMLoadFloat3(&vMinf3);
				XMVECTOR meshMax = XMLoadFloat3(&vMaxf3);

				const uint vCount = vertexCount[i];
				objMeshData[i].staticMeshData.vertices.resize(vCount);

				for (uint j = 0; j < vCount; ++j)
				{
					objMeshData[i].staticMeshData.vertices[j] = vertex[vertexCount[i] + j];

					const XMVECTOR p = XMLoadFloat3(&objMeshData[i].staticMeshData.vertices[j].position);
					meshMin = XMVectorMin(meshMin, p);
					meshMax = XMVectorMax(meshMax, p);
				}
				modelMin = XMVectorMin(modelMin, meshMin);
				modelMax = XMVectorMax(modelMax, meshMax);

				objMeshData[i].boundingBox = JDirectXCollisionEx::CreateBoundingBox(meshMin, meshMax);
				objMeshData[i].boundingSphere = JDirectXCollisionEx::CreateBoundingSphere(meshMin, meshMax);
			}
			if (hasTangent && isSameFace)
			{
				std::vector<XMFLOAT4> tangent(vertex.size());
				for (int i = 0; i < meshCount; ++i)
				{
					normal[i] = vertex[i].normal;
					texture[i] = vertex[i].texC;
				}

				ComputeTangentFrame(index.data(),
					(uint)faceInfo[0][0].ptn.size(),
					position.data(),
					normal.data(),
					texture.data(),
					3,
					tangent.data());

				for (int i = 0; i < meshCount; ++i)
				{
					const uint vCount = vertexCount[i];
					for (uint j = 0; j < vCount; ++j)
					{
						objMeshData[i].staticMeshData.vertices[j].tangentU.x = tangent[vCount + j].x;
						objMeshData[i].staticMeshData.vertices[j].tangentU.y = tangent[vCount + j].y;
						objMeshData[i].staticMeshData.vertices[j].tangentU.z = tangent[vCount + j].z;
					}
				}
			}
			modelAttribute.modelBBox = JDirectXCollisionEx::CreateBoundingBox(modelMin, modelMax);
			modelAttribute.modelBSphere = JDirectXCollisionEx::CreateBoundingSphere(modelMin, modelMax);
			modelAttribute.totalIndex = (uint)index.size();
			modelAttribute.totalVertex = (uint)vertex.size();
			modelAttribute.hasSkeleton = false;

			return true;
		}
		bool JObjFileLoaderImpl::LoadMatFile(const std::wstring& path, std::vector<JObjMatData>& objMatData)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in, std::ios::binary);
			if (!stream.is_open())
				return false;

			float x, y, z;

			int matCount = -1;
			std::wstring next;
			while (stream >> next)
			{
				if (next == L"newmtl")
				{
					std::getline(stream, next);
					objMatData.emplace_back(JObjMatData());
					++matCount;
					objMatData[matCount].name = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
				}
				else if (next == L"Ka")
				{
					stream >> x >> y >> z;
					objMatData[matCount].ambient = XMFLOAT4(x, y, z, 1);
				}
				else if (next == L"Kd")
				{
					stream >> x >> y >> z;
					objMatData[matCount].albedo = XMFLOAT4(x, y, z, 1);
				}
				else if (next == L"Ks")
				{
					stream >> x >> y >> z;
					objMatData[matCount].specular = XMFLOAT4(x, y, z, 1);
				}
				else if (next == L"Ns")
				{
					stream >> x;
					objMatData[matCount].specularWeight = x;
				}
				else if (next == L"d")
				{
					stream >> x;
					objMatData[matCount].opaqueFactor = x;
				}
				else if (next == L"Tr")
				{
					stream >> x;
					objMatData[matCount].opaqueFactor = 1 - x;
				}
				else if (next == L"Ni")
				{
					stream >> x;
					objMatData[matCount].opticalDensity = x;
				}
				else if (next == L"map_Ka")
				{
					std::getline(stream, next);
					objMatData[matCount].ambientTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T);
				}
				else if (next == L"map_Kd")
				{
					std::getline(stream, next);
					objMatData[matCount].albedoTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_ALBEDO_T);
				}
				else if (next == L"map_Ks")
				{
					std::getline(stream, next);
					objMatData[matCount].specularColorTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_SPECULAR_COLOR_T);
				}
				else if (next == L"map_Ns")
				{
					std::getline(stream, next);
					objMatData[matCount].specularHighlightTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_SPECULAR_HIGHLIGHT_T);
				}
				else if (next == L"map_d")
				{
					std::getline(stream, next);
					objMatData[matCount].ambientTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T);
				}
				else if (next == L"map_bump" || next == L"bump")
				{
					std::getline(stream, next);
					objMatData[matCount].normalTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_NORMAL_T);
				}
				else if (next == L"disp")
				{
					std::getline(stream, next);
					objMatData[matCount].heightTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_HEIGHT_T);
				}
				else if (next == L"decal")
				{
					std::getline(stream, next);
					objMatData[matCount].decalTName = JCommonUtility::EraseEmptySpace(JCommonUtility::WstringToU8String(next));
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_DECAL_T);
				}
			}
			stream.close();
			return true;
		}
		void JObjFileLoaderImpl::GetVectorIndex(const std::wstring& wstr, int& posIndex, int& uvIndex, int& normalIndex)const noexcept
		{
			int firstIndex = (int)wstr.find_first_of('/');
			int secondIndex = (int)wstr.substr(firstIndex + 1).find_first_of('/');
			if (secondIndex == -1)
			{
				//	"V/T"
				posIndex = JCommonUtility::WStringToInt(wstr.substr(0, firstIndex));
				uvIndex = JCommonUtility::WStringToInt(wstr.substr(firstIndex + 1));
				normalIndex = -1;
			}
			else
			{
				if (secondIndex == 0)
				{
					//	"V//N"
					posIndex = JCommonUtility::WStringToInt(wstr.substr(0, firstIndex));
					uvIndex = -1;
					normalIndex = JCommonUtility::WStringToInt(wstr.substr(secondIndex + 1));
				}
				else
				{
					//"V/T/V"
					posIndex = JCommonUtility::WStringToInt(wstr.substr(0, firstIndex));
					uvIndex = JCommonUtility::WStringToInt(wstr.substr(firstIndex + 1, secondIndex));
					normalIndex = JCommonUtility::WStringToInt(wstr.substr(secondIndex + 1));
				}
			}
		}
		std::string JObjFileLoaderImpl::GetMaterialPath(const std::string& folderPath, const std::string& name)const noexcept
		{
			return folderPath + name + u8".mtl";
		}
	}
}
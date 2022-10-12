#include"JObjFileLoader.h"
#include"../../DirectXEx/JDirectXCollisionEx.h"
#include"../../Guid/GuidCreator.h"
#include"../../File/JFilePathData.h"
#include"../../../Object/Resource/Mesh/JMeshStruct.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Resource/JResourcePathData.h"
#include"../../../Object/Resource/Model/JModelAttribute.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Utility/JMathHelper.h"

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

		bool JObjFileLoaderImpl::LoadObjFile(const JFileImportPathData& pathData, JObjFileMeshData& objMeshData, std::vector<JObjFileMatData>& objMatData)
		{
			std::wstring next;
			std::wstring materialLibName;
			std::vector<std::wstring> meshName;
			std::vector<std::wstring> materialName;
			std::vector<XMFLOAT3> position;
			std::vector<XMFLOAT2> texture;
			std::vector<XMFLOAT3> normal;
			std::vector<uint> index;
			std::vector<uint>vertexCount;
			std::vector<uint>indexCount;
			std::vector<std::vector<FaceInfo>> faceInfo;

			float x, y, z;
			int meshCount = -1;
			int faceCount =0;
			std::wifstream stream;

			stream.open(pathData.oriFileWPath, std::ios::in, std::ios::binary);
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
					meshName.push_back(JCUtil::EraseSideWChar(next, L' '));
					materialName.push_back(L" ");
					vertexCount.push_back(0);
					indexCount.push_back(0);
					faceInfo.emplace_back(std::vector<FaceInfo>());
					++meshCount;
				}
				else if (next == L"usemtl")
				{
					std::getline(stream, next);
					materialName[meshCount] = JCUtil::EraseSideWChar(next, L' ');
				}
				else if (next == L"mtllib")
				{
					std::getline(stream, next);
					materialLibName = JCUtil::EraseSideWChar(next, L' ');
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
					next = JCUtil::EraseSideWChar(next, L' ');

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
					++faceCount;
					indexCount[meshCount] += nextCount;
				}
			}
			stream.close();

			bool hasMat = false;
			if (!materialLibName.empty())
				hasMat = LoadMatFile(materialLibName, objMatData);

			std::vector<JStaticMeshVertex> totalVertex(position.size());
			std::vector<std::vector<uint32>> meshIndicies(meshCount);
		 

			for (int i = 0; i < meshCount; ++i)
			{
				JStaticMeshData meshdata;
				objMeshData.materialName.push_back(materialName[i]);
				meshIndicies[i].resize(indexCount[i]);

				uint indexSum = 0;
				const int ptnIndexInfoCount = (int)faceInfo[i].size();
				for (int j = 0; j < ptnIndexInfoCount; ++j)
				{
					uint fCount = (uint)faceInfo[i][j].ptn.size();
					for (uint k = 0; k < fCount; ++k)
					{
						const int pIndex = faceInfo[i][j].ptn[k].x;
						const int nIndex = faceInfo[i][j].ptn[k].y;
						const int tIndex = faceInfo[i][j].ptn[k].z;
					 
						totalVertex[pIndex] = JStaticMeshVertex(position[pIndex], { 0,0,0 }, {0, 0});
						if (nIndex != -1)
							totalVertex[pIndex].normal = normal[nIndex];
						if (tIndex != -1)
							totalVertex[pIndex].texC = texture[tIndex];

						meshIndicies[i][indexSum] = pIndex - indexCount[i];
						++indexSum;
					}
				}
			}

			bool hasTangent = true;
			bool isSameFace = true;

			for (int i = 0; i < meshCount; ++i)
			{
				const uint vCount = vertexCount[i];
				std::vector<JStaticMeshVertex> meshVertex(vCount);

				for (uint j = 0; j < vCount; ++j)
					meshVertex[j] = totalVertex[vertexCount[i] + j];
				 
				objMeshData.meshGroup.AddMeshData(JStaticMeshData{meshName[i],
					MakeGuid(), 
					std::move(meshIndicies[i]),
					faceInfo[i][0].hasNormal,
					faceInfo[i][0].hasUV, 
					std::move(meshVertex)});
			}
			if (hasTangent && isSameFace)
			{
				std::vector<XMFLOAT4> tangent(totalVertex.size());
				for (int i = 0; i < meshCount; ++i)
				{
					normal[i] = totalVertex[i].normal;
					texture[i] = totalVertex[i].texC;
				}

				ComputeTangentFrame(index.data(),
					faceCount,
					position.data(),
					normal.data(),
					texture.data(),
					position.size(),
					tangent.data());

				for (int i = 0; i < meshCount; ++i)
				{
					JStaticMeshData* meshdata = static_cast<JStaticMeshData*>(objMeshData.meshGroup.GetMeshData(i));
					const int vCount = (int)vertexCount[i];
					for (int j = 0; j < vCount; ++j)
					{
						JStaticMeshVertex vertex = meshdata->GetVertex(j);
						vertex.tangentU.x = tangent[vCount + j].x;
						vertex.tangentU.y = tangent[vCount + j].y;
						vertex.tangentU.z = tangent[vCount + j].z;
						meshdata->SetVertex(j, vertex);
					}
				}
			}
			return true;
		}
		bool JObjFileLoaderImpl::LoadMatFile(const std::wstring& path, std::vector<JObjFileMatData>& objMatData)
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
					objMatData.emplace_back(JObjFileMatData());
					++matCount;
					objMatData[matCount].name = JCUtil::EraseSideWChar(next, L' ');
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
					objMatData[matCount].ambientTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T);
				}
				else if (next == L"map_Kd")
				{
					std::getline(stream, next);
					objMatData[matCount].albedoTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_ALBEDO_T);
				}
				else if (next == L"map_Ks")
				{
					std::getline(stream, next);
					objMatData[matCount].specularColorTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_SPECULAR_COLOR_T);
				}
				else if (next == L"map_Ns")
				{
					std::getline(stream, next);
					objMatData[matCount].specularHighlightTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_SPECULAR_HIGHLIGHT_T);
				}
				else if (next == L"map_d")
				{
					std::getline(stream, next);
					objMatData[matCount].ambientTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T);
				}
				else if (next == L"map_bump" || next == L"bump")
				{
					std::getline(stream, next);
					objMatData[matCount].normalTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_NORMAL_T);
				}
				else if (next == L"disp")
				{
					std::getline(stream, next);
					objMatData[matCount].heightTName = JCUtil::EraseSideWChar(next, L' ');
					objMatData[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)objMatData[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_HEIGHT_T);
				}
				else if (next == L"decal")
				{
					std::getline(stream, next);
					objMatData[matCount].decalTName = JCUtil::EraseSideWChar(next, L' ');
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
				posIndex = (int)JCUtil::WstringToInt(wstr.substr(0, firstIndex));
				uvIndex = (int)JCUtil::WstringToInt(wstr.substr(firstIndex + 1));
				normalIndex = -1;
			}
			else
			{
				if (secondIndex == 0)
				{
					//	"V//N"
					posIndex = (int)JCUtil::WstringToInt(wstr.substr(0, firstIndex));
					uvIndex = -1;
					normalIndex = (int)JCUtil::WstringToInt(wstr.substr(secondIndex + 1));
				}
				else
				{
					//"V/T/V"
					posIndex = (int)JCUtil::WstringToInt(wstr.substr(0, firstIndex));
					uvIndex = (int)JCUtil::WstringToInt(wstr.substr(firstIndex + 1, secondIndex));
					normalIndex = (int)JCUtil::WstringToInt(wstr.substr(secondIndex + 1));
				}
			}
		}
		std::wstring JObjFileLoaderImpl::GetMaterialPath(const std::wstring& folderPath, const std::wstring& name)const noexcept
		{
			return folderPath + name + L".mtl";
		}
	}
}
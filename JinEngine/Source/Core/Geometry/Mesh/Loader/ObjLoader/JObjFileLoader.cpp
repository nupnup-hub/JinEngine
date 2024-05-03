#include"JObjFileLoader.h"
#include"../../JMeshStruct.h"
#include"../../../JDirectXCollisionEx.h"
#include"../../../../Guid/JGuidCreator.h"
#include"../../../../File/JFilePathData.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Math/JMathHelper.h"   
#include"../../../../Log/JLogMacro.h"   
#include"../../../../ThirdParty/DirectX/DirectXMesh.h" 
#include<fstream>
#include<io.h>

//수정필요
namespace JinEngine
{
	namespace Core
	{
		JObjFileLoader::FaceInfo::FaceInfo(const std::vector<JVector3<int>>& ptn)
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

		bool JObjFileLoader::LoadObjFile(const JFileImportHelpData& pathData, JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData)
		{
			set = std::make_unique<LoadDataSet>();
			bool success = LoadMesh(pathData);
			if (success)
			{
				set->hasMaterial = LoadMaterial();
				success = CreateMeshGroup(meshGroup, objMatData);
			}
			set = nullptr;
			return success;
		}
		bool JObjFileLoader::LoadMesh(const JFileImportHelpData& pathData)
		{
			std::wifstream stream;
			stream.open(pathData.oriFileWPath, std::ios::in, std::ios::binary);
			if (!stream.is_open())
			{
				J_LOG_PRINT_OUT("", "Stream fail");
				stream.close();
				return false;
			}

			std::wstring next;
			float x, y, z; 

			while (stream >> next)
			{
				if (next == L"o" || next == L"object" || next == L"Object")
				{
					std::getline(stream, next);
					set->meshName.push_back(JCUtil::EraseSideWChar(next, L' '));
					set->materialName.push_back(L" ");
					set->vertexCount.push_back(0);
					set->indexCount.push_back(0);
					set->faceInfo.emplace_back(std::vector<FaceInfo>());
					++set->meshCount;
				}
				else if (next == L"usemtl")
				{
					std::getline(stream, next);
					set->materialName[set->meshCount - 1] = JCUtil::EraseSideWChar(next, L' ');
				}
				else if (next == L"mtllib")
				{
					std::getline(stream, next);
					set->materialLibName = JCUtil::EraseSideWChar(next, L' ');
				}
				else if (next == L"v")
				{
					stream >> x >> y >> z;
					DetectNan(x, y, z);
					set->position.emplace_back(SetLeftHand(JVector3<float>(x, y, z)));
					++set->vertexCount[set->meshCount - 1];
				}
				else if (next == L"vt")
				{
					stream >> x >> y;
					DetectNan(x, y);
					set->texture.emplace_back(x, y);
				}
				else if (next == L"vn")
				{
					stream >> x >> y >> z;
					DetectNan(x, y, z);
					set->normal.emplace_back(SetLeftHand(JVector3<float>(x, y, z)));
				}
				else if (next == L"f")
				{
					std::getline(stream, next);
					next = JCUtil::EraseSideWChar(next, L' ');
					std::vector<JVector3<int>> ptn;
					while (next.size() > 0)
					{
						int spaceIndex = next.find_first_of(' ');
						int posIndex = 0;
						int uvIndex = 0;
						int normalIndex = 0;

						if (spaceIndex == -1)
						{
							GetVectorIndex(next, posIndex, uvIndex, normalIndex);
							next.clear();
						}
						else
						{
							GetVectorIndex(next.substr(0, spaceIndex), posIndex, uvIndex, normalIndex);
							next = next.substr(spaceIndex + 1);
						}
						ptn.emplace_back(posIndex, uvIndex, normalIndex);
						set->index.push_back(posIndex);
					}
					if (ptn.size() > 0)
					{
						set->faceInfo[set->meshCount - 1].emplace_back(FaceInfo(ptn));
						++set->faceCount;
						set->indexCount[set->meshCount - 1] += ptn.size();
					}
				}
			}
			stream.close();
			return true;
		}
		bool JObjFileLoader::LoadMaterial()
		{
			if (set->materialLibName.empty())
				return false;

			std::wifstream stream;
			stream.open(set->materialLibName, std::ios::in, std::ios::binary);
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
					set->materialVec.emplace_back(JObjFileMaterial());
					++matCount;
					set->materialVec[matCount].name = JCUtil::EraseSideWChar(next, L' ');
					set->materialMap.emplace(set->materialVec[matCount].name, set->materialVec[matCount]);
				}
				else if (next == L"Ka")
				{
					stream >> x >> y >> z;
					set->materialVec[matCount].ambient = JVector4<float>(x, y, z, 1);
				}
				else if (next == L"Kd")
				{
					stream >> x >> y >> z;
					set->materialVec[matCount].mParam.albedoColor = JVector4<float>(x, y, z, 1);
				}
				else if (next == L"Ks")
				{
					stream >> x >> y >> z;
					set->materialVec[matCount].mParam.specularFactor = x;
				}
				else if (next == L"Ns")
				{
					stream >> x;
					set->materialVec[matCount].specularWeight = x;
				}
				else if (next == L"d")
				{
					stream >> x;
					set->materialVec[matCount].opaqueFactor = x;
				}
				else if (next == L"Tr")
				{
					stream >> x;
					set->materialVec[matCount].opaqueFactor = 1 - x;
				}
				else if (next == L"Ni")
				{
					stream >> x;
					set->materialVec[matCount].opticalDensity = x;
				}
				else if (next == L"map_Ka")
				{
					std::getline(stream, next);
					set->materialVec[matCount].ambientMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T);
				}
				else if (next == L"map_Kd")
				{
					std::getline(stream, next);
					set->materialVec[matCount].albedoMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_ALBEDO_T);
				}
				else if (next == L"map_Ks")
				{
					std::getline(stream, next);
					set->materialVec[matCount].specularColorMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_SPECULAR_COLOR_T);
				}
				else if (next == L"map_Ns")
				{
					std::getline(stream, next);
					set->materialVec[matCount].specularHighlightMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_SPECULAR_HIGHLIGHT_T);
				}
				else if (next == L"map_d")
				{
					std::getline(stream, next);
					set->materialVec[matCount].ambientMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T);
				}
				else if (next == L"map_bump" || next == L"bump")
				{
					std::getline(stream, next);
					set->materialVec[matCount].normalMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_NORMAL_T);
				}
				else if (next == L"disp")
				{
					std::getline(stream, next);
					set->materialVec[matCount].heightMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_HEIGHT_T);
				}
				else if (next == L"decal")
				{
					std::getline(stream, next);
					set->materialVec[matCount].decalMapName = JCUtil::EraseSideWChar(next, L' ');
					set->materialVec[matCount].flag = (JOBJ_MATERIAL_FLAG)((int)set->materialVec[matCount].flag | (int)JOBJ_MATERIAL_FLAG::HAS_DECAL_T);
				}
			}
			stream.close();
			return true;
		}
		bool JObjFileLoader::CreateMeshGroup(JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData)
		{ 		    
			for (int i = 0; i < set->meshCount; ++i)
			{
				const uint iCount = set->indexCount[i];
				const uint vCount = set->vertexCount[i];

				std::vector<uint32>meshIndicies;
				std::vector<JStaticMeshVertex> meshVertex;

				const int ptnIndexInfoCount = (int)set->faceInfo[i].size();
				if (ptnIndexInfoCount == 0)
					continue;

				for (int j = 0; j < ptnIndexInfoCount; ++j)
				{
					static constexpr uint maxFaceCount = 4;
					DirectX::XMFLOAT3 positionXm[maxFaceCount];
					DirectX::XMFLOAT2 textureXm[maxFaceCount];
					DirectX::XMFLOAT3 normalXm[maxFaceCount];
					DirectX::XMFLOAT4 tangentXm[maxFaceCount]; 

					uint fCount = (uint)set->faceInfo[i][j].ptn.size();
					uint indexOffset = (uint)meshIndicies.size();
					for (uint k = 0; k < fCount; ++k)
					{
						//Index 1..n
						const int pIndex = set->faceInfo[i][j].ptn[k].x - 1;
						const int tIndex = set->faceInfo[i][j].ptn[k].y - 1;
						const int nIndex = set->faceInfo[i][j].ptn[k].z - 1;

						positionXm[k] = set->position[pIndex].ToSimilar<DirectX::XMFLOAT3 >();
						if (tIndex >= 0)
							textureXm[k] = set->texture[tIndex].ToSimilar<DirectX::XMFLOAT2>();
						if (nIndex >= 0)
							normalXm[k] = set->normal[nIndex].ToSimilar<DirectX::XMFLOAT3 >();

						size_t guid = MakeVertexGuid(pIndex, tIndex, nIndex);
						auto vIndexData = set->vertexIndexMap.find(guid);
						if (vIndexData != set->vertexIndexMap.end())
							meshIndicies.push_back(vIndexData->second);
						else
						{
							JStaticMeshVertex newVertex;
							newVertex.position = set->position[pIndex];
							if (tIndex >= 0)
								newVertex.texC = set->texture[tIndex];
							if (nIndex >= 0)
								newVertex.normal = set->normal[nIndex];

							uint vertexIndex = (uint)meshVertex.size();
							meshIndicies.push_back(vertexIndex);
							meshVertex.push_back(newVertex);
							set->vertexIndexMap.emplace(guid, vertexIndex);
						}
					}
					if (set->faceInfo[i][j].hasNormal && set->faceInfo[i][j].hasUV && false)
					{
						DirectX::ComputeTangentFrame(&meshIndicies[indexOffset],
							1,
							positionXm,
							normalXm,
							textureXm,
							fCount,
							tangentXm);
						for (uint k = 0; k < fCount; ++k)
						{
							uint vIndex = meshIndicies[k + indexOffset];
							meshVertex[vIndex].tangentU = JVector3F(tangentXm[k].x, tangentXm[k].y, tangentXm[k].z);
						}
					}

				} 
				std::unique_ptr<JStaticMeshData> staticMeshData = std::make_unique<JStaticMeshData>(set->meshName[i],
					MakeGuid(),
					std::move(meshIndicies),
					set->faceInfo[i][0].hasNormal,
					set->faceInfo[i][0].hasUV,
					std::move(meshVertex));

				if (set->hasMaterial)
				{
					auto matData = set->materialMap.find(set->materialName[i]);
					if (matData != set->materialMap.end())
						objMatData.emplace(staticMeshData->GetGuid(), matData->second);
				} 
				meshGroup.AddMeshData(std::move(staticMeshData)); 
			} 
			/*
			
			bool hasTangent = true;
			bool isSameFace = true;
			if (hasTangent && isSameFace)
			{
				std::vector<JVector4<float>> tangent(totalVertex.size());
				const uint totalVertexCount = (uint)totalVertex.size();
				for (int i = 0; i < totalVertexCount; ++i)
				{
					normal[i] = totalVertex[i].normal;
					texture[i] = totalVertex[i].texC;
				}
		  
				vertexOffset = 0;
				for (int i = 0; i < meshCount; ++i)
				{
					JStaticMeshData* meshdata = static_cast<JStaticMeshData*>(objMeshData.meshGroup.GetMeshData(i));
					const int vCount = (int)vertexCount[i];

					for (int j = 0; j < vCount; ++j)
					{
						JStaticMeshVertex vertex = meshdata->GetVertex(j);
						vertex.tangentU.x = tangent[vertexOffset + j].x;
						vertex.tangentU.y = tangent[vertexOffset + j].y;
						vertex.tangentU.z = tangent[vertexOffset + j].z;
						meshdata->SetVertex(j, vertex);
					}
					vertexOffset += vCount;
				}
			}
			*/
			/*
			Debug
			std::wofstream dStream;
			dStream.open(L"D:\\JinWooJung\\LoadObjDebug.txt", std::ios::out);


			JFileIOHelper::StoreAtomicData(dStream, L"pIndex:", pIndex);
			JFileIOHelper::StoreAtomicData(dStream, L"nIndex:", nIndex);
			JFileIOHelper::StoreXMFloat3(dStream, L"normal:", normal[nIndex]);
			JFileIOHelper::InputSpace(dStream, 1);

			for (uint i = 0; i < totalVertex.size(); ++i)
			{
				JFileIOHelper::StoreAtomicData(dStream, L"Index:", i);
				JFileIOHelper::StoreXMFloat3(dStream, L"Normal:", totalVertex[i].normal);
				JFileIOHelper::StoreAtomicData(dStream, L"IsNanX:", isnan(totalVertex[i].normal.x));
				JFileIOHelper::StoreAtomicData(dStream, L"IsNanY:", isnan(totalVertex[i].normal.y));
				JFileIOHelper::StoreAtomicData(dStream, L"IsNanZ:", isnan(totalVertex[i].normal.z));
				JFileIOHelper::InputSpace(dStream, 1);
			}
			dStream.close();
			*/
			return true;
		}
		void JObjFileLoader::GetVectorIndex(const std::wstring& wstr, int& posIndex, int& uvIndex, int& normalIndex)const noexcept
		{
			int firstIndex = (int)wstr.find_first_of('/');
			if (firstIndex == -1)
			{
				//V
				posIndex = (int)JCUtil::WstringToInt(wstr);
			}
			else
			{
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
						normalIndex = (int)JCUtil::WstringToInt(wstr.substr(secondIndex + firstIndex + 2));
					}
					else
					{
						//"V/T/V" 
						posIndex = (int)JCUtil::WstringToInt(wstr.substr(0, firstIndex));
						uvIndex = (int)JCUtil::WstringToInt(wstr.substr(firstIndex + 1, secondIndex));
						normalIndex = (int)JCUtil::WstringToInt(wstr.substr(secondIndex + firstIndex + 2));
					}
				}
			}
		}
		std::wstring JObjFileLoader::GetMaterialPath(const std::wstring& folderPath, const std::wstring& name)const noexcept
		{
			return folderPath + name + L".mtl";
		}
		void JObjFileLoader::DetectNan(float& x, float& y)
		{
			x = std::isnan(x) || std::isinf(x) ? 0 : x;
			y = std::isnan(y) || std::isinf(y) ? 0 : y;
		}
		void JObjFileLoader::DetectNan(float& x, float& y, float& z)
		{
			x = std::isnan(x) || std::isinf(x) ? 0 : x;
			y = std::isnan(y) || std::isinf(y) ? 0 : y;
			z = std::isnan(z) || std::isinf(z) ? 0 : z;
		}
		JVector3<float> JObjFileLoader::SetLeftHand(const JVector3<float>& v)noexcept
		{
			//수정필요
			return JVector3<float>(v.x, v.y, v.z);
			//return JVector3<float>(v.x, v.y, v.z);
		}
		size_t JObjFileLoader::MakeVertexGuid(const uint x, const uint y, const uint z)
		{
			return JCUtil::CalculateGuid("x:" + std::to_string(x) +
				"y:" + std::to_string(y) +
				"z:" + std::to_string(z));
		}
	}
}
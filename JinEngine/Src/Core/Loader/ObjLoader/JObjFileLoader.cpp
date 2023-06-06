#include"JObjFileLoader.h"
#include"../../Geometry/JDirectXCollisionEx.h"
#include"../../Guid/JGuidCreator.h"
#include"../../File/JFilePathData.h"
//#include"../../File/JFileIOHelper.h"
#include"../../../Object/Resource/Mesh/JMeshStruct.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Utility/JCommonUtility.h"
#include"../../../Utility/JMathHelper.h"

#include<fstream>
#include<io.h>

//수정필요
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

		bool JObjFileLoaderImpl::LoadObjFile(const JFileImportHelpData& pathData, JObjFileMeshData& objMeshData, std::vector<JObjFileMatData>& objMatData)
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
			int meshCount = 0;
			int faceCount = 0;
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
					materialName[meshCount - 1] = JCUtil::EraseSideWChar(next, L' ');
				}
				else if (next == L"mtllib")
				{
					std::getline(stream, next);
					materialLibName = JCUtil::EraseSideWChar(next, L' ');
				}
				else if (next == L"v")
				{
					stream >> x >> y >> z;
					DetectNan(x, y, z);
					position.emplace_back(SetLeftHand(DirectX::XMFLOAT3(x,y,z)));
					++vertexCount[meshCount - 1];
				}
				else if (next == L"vt")
				{
					stream >> x >> y;
					DetectNan(x, y);
					texture.emplace_back(x, y);
				}
				else if (next == L"vn")
				{
					stream >> x >> y >> z;
					DetectNan(x, y, z);
					normal.emplace_back(SetLeftHand(DirectX::XMFLOAT3(x, y, z))); 
				}
				else if (next == L"f")
				{
					std::getline(stream, next);
					next = JCUtil::EraseSideWChar(next, L' ');
					std::vector<XMINT3> ptn;
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
						index.push_back(posIndex);
					} 
					faceInfo[meshCount - 1].emplace_back(FaceInfo(ptn));
					++faceCount;
					indexCount[meshCount - 1] += ptn.size();
				}
			}
			stream.close();

			bool hasMat = false;
			if (!materialLibName.empty())
				hasMat = LoadMatFile(materialLibName, objMatData);

			std::unordered_map<size_t, uint> vertexIndexMap;
			int vertexOffset = 0;

			for (int i = 0; i < meshCount; ++i)
			{
				objMeshData.materialName.push_back(materialName[i]);

				const uint iCount = indexCount[i];
				const uint vCount = vertexCount[i];

				std::vector<uint32>meshIndicies;
				std::vector<JStaticMeshVertex> meshVertex;
				 
				const int ptnIndexInfoCount = (int)faceInfo[i].size();
				for (int j = 0; j < ptnIndexInfoCount; ++j)
				{
					uint fCount = (uint)faceInfo[i][j].ptn.size();
					for (uint k = 0; k < fCount; ++k)
					{
						//Index 1..n
						const int pIndex = faceInfo[i][j].ptn[k].x - 1;
						const int tIndex = faceInfo[i][j].ptn[k].y - 1;
						const int nIndex = faceInfo[i][j].ptn[k].z - 1;

						size_t guid = MakeVertexGuid(pIndex, tIndex, nIndex);
						auto vIndexData = vertexIndexMap.find(guid);
						if (vIndexData != vertexIndexMap.end())
							meshIndicies.push_back(vIndexData->second);
						else
						{
							JStaticMeshVertex newVertex;
							newVertex.position = position[pIndex];
							if (tIndex >= 0)
								newVertex.texC = texture[tIndex];
							if (nIndex >= 0)
								newVertex.normal = normal[nIndex];

							uint vertexIndex = (uint)meshVertex.size();
							meshIndicies.push_back(vertexIndex);
							meshVertex.push_back(newVertex);
							vertexIndexMap.emplace(guid, vertexIndex);
						}
					}
				} 
				JStaticMeshData staticMeshData(meshName[i],
					MakeGuid(),
					std::move(meshIndicies),
					faceInfo[i][0].hasNormal,
					faceInfo[i][0].hasUV,
					std::move(meshVertex));

				objMeshData.meshGroup.AddMeshData(std::move(staticMeshData));
				vertexOffset += vCount;
			}
			bool hasTangent = true;
			bool isSameFace = true;
			if (hasTangent && isSameFace)
			{
				 /*
				 std::vector<XMFLOAT4> tangent(totalVertex.size());
				const uint totalVertexCount = (uint)totalVertex.size();
				for (int i = 0; i < totalVertexCount; ++i)
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
				 */
			}
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
		std::wstring JObjFileLoaderImpl::GetMaterialPath(const std::wstring& folderPath, const std::wstring& name)const noexcept
		{
			return folderPath + name + L".mtl";
		}
		void JObjFileLoaderImpl::DetectNan(float& x, float& y)
		{
			x = std::isnan(x) || std::isinf(x) ? 0 : x;
			y = std::isnan(y) || std::isinf(y) ? 0 : y;
		}
		void JObjFileLoaderImpl::DetectNan(float& x, float& y, float& z)
		{
			x = std::isnan(x) || std::isinf(x) ? 0 : x;
			y = std::isnan(y) || std::isinf(y) ? 0 : y;
			z = std::isnan(z) || std::isinf(z) ? 0 : z;
		}
		DirectX::XMFLOAT3 JObjFileLoaderImpl::SetLeftHand(const DirectX::XMFLOAT3& v)noexcept
		{ 
			//수정필요
			return DirectX::XMFLOAT3(v.x, v.y, v.z);
			//return DirectX::XMFLOAT3(v.x, v.y, v.z);
		}
		size_t JObjFileLoaderImpl::MakeVertexGuid(const uint x, const uint y, const uint z)
		{
			return JCUtil::CalculateGuid("x:" + std::to_string(x) +
				"y:" + std::to_string(y) + 
				"z:" + std::to_string(z));
		}
	}
}
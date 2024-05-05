#include"JFbxFileLoader.h" 
#include"../../../JDirectXCollisionEx.h"
#include"../../../../Guid/JGuidCreator.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Math/JMathHelper.h"   
#include"../../../../Animation/Joint.h"  
#include"../../../../../Core/Identity/JIdenCreator.h"
#include"../../../../../Core/Log/JLogMacro.h"
//#include"../../../../../Develop/Debug/JDevelopDebug.h"
//#include"../../File/JFileIOHelper.h"
using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{  
		namespace Private
		{
			static JVector4F ZUpToYUpxMinus90()noexcept
			{
				return { 0.707107f, 0.0f, 0.0f, -0.707107f };
			}
			static JMatrix4x4 YUpRToYUpL()noexcept
			{
				return { -1,0,0,0,
					0,1,0,0,
					0,0,1,0,
					0,0,0,1 };
			}
			static JMatrix4x4 ZUpRToYUpL()noexcept
			{
				return {-1,0,0,0,
					0,0,-1,0,
					0,1,0,0,
					0,0,0,1};
			}
			static JVector3F Convert(const FbxPropertyT<FbxDouble3>& d)noexcept
			{ 
				return JVector3F(d.Get()[0], d.Get()[1], d.Get()[2]);
			}
			static constexpr int failUpDirSearch = -2;
		}
		JFbxFileLoader::~JFbxFileLoader()
		{
			if (fbxManager != nullptr)
			{
				fbxManager->Destroy();
			}
		}
		J_FBX_RESULT JFbxFileLoader::LoadFbxMeshFile(const std::string& path, JStaticMeshGroup& meshGroup, JFbxMaterialMap& materialMap)
		{
			if (fbxManager == nullptr)
			{
				fbxManager = FbxManager::Create();
				FbxIOSettings* fbxIoSetting = FbxIOSettings::Create(fbxManager, IOSROOT);
				fbxManager->SetIOSettings(fbxIoSetting);
			}
			FbxImporter* importer = FbxImporter::Create(fbxManager, "");
			bool status = importer->Initialize(path.c_str(), -1, fbxManager->GetIOSettings());
			if (!status)
			{
				printf("Call to Fbx::Initialize() failed.\n");
				printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
				return J_FBX_RESULT_FAIL;
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");

			//fbx file 내용을 scene으로 가져온다
			if (importer->Import(scene))
			{
				//++num; 
				FbxGlobalSettings& settings = scene->GetGlobalSettings();

				dataSet = std::make_unique<FbxLoadDataSet>();
				dataSet->resizeRate = 0.01f / settings.GetSystemUnit().GetMultiplier();
				//settings.SetSystemUnit(FbxSystemUnit::cm);
				//settings.GetSystemUnit().ConvertScene(scene);
				//resizeRate = 0.01f;
				//resizeRate = 0.1f / settings.GetSystemUnit().GetScaleFactor();
	 
				//settings.GetSystemUnit().GetScaleFactor()
				// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
				FbxNode* rootNode = scene->GetRootNode();
				SetSceneAxis(scene, rootNode);

				FbxGeometryConverter geometryConverter(fbxManager);
				geometryConverter.Triangulate(scene, true);
				J_FBX_RESULT result;
				  
				LoadMaterial(scene);
				LoadNode(rootNode, meshGroup, materialMap);
				if (meshGroup.GetMeshDataCount() > 0)
					result = J_FBX_RESULT_HAS_MESH;
				 
				scene->Destroy();
				importer->Destroy();
				dataSet = nullptr;
				return result;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return J_FBX_RESULT_FAIL;
			}
		}
		J_FBX_RESULT JFbxFileLoader::LoadFbxMeshFile(const std::string& path, JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint, JFbxMaterialMap& materialMap)
		{
			if (fbxManager == nullptr)
			{ 
				fbxManager = FbxManager::Create();
				FbxIOSettings* fbxIoSetting = FbxIOSettings::Create(fbxManager, IOSROOT);
				fbxManager->SetIOSettings(fbxIoSetting);
			}
			FbxImporter* importer = FbxImporter::Create(fbxManager, "");
			bool status = importer->Initialize(path.c_str(), -1, fbxManager->GetIOSettings());
			if (!status)
			{
				printf("Call to Fbx::Initialize() failed.\n");
				printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
				return J_FBX_RESULT_FAIL;
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");
		 
			//fbx file 내용을 scene으로 가져온다
			if (importer->Import(scene))
			{ 
				FbxGlobalSettings& settings = scene->GetGlobalSettings();

				dataSet = std::make_unique<FbxLoadDataSet>();
				dataSet->resizeRate = 0.01f / settings.GetSystemUnit().GetMultiplier();
				//settings.SetSystemUnit(FbxSystemUnit::cm);
				//settings.GetSystemUnit().ConvertScene(scene);
				//resizeRate = 0.01f;
 
				//++num; 
				// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
				FbxNode* rootNode = scene->GetRootNode();
				SetSceneAxis(scene, rootNode);
				 
				FbxGeometryConverter geometryConverter(fbxManager);
				geometryConverter.Triangulate(scene, true);
				J_FBX_RESULT result;
				 
				//hasRootJoint = true;
				LoadMaterial(scene);

				JFbxSkeleton fbxSkeleton;
				LoadJoint(rootNode, -1, -1, -1, fbxSkeleton);

				//ProcessSkeletonHierarchy(rootNode, fbxSkeleton);
				//fbxSkeleton.joint[0].parentIndex == 0 add emptyNode in front
				bool hasSkeleton = fbxSkeleton.jointCount > 0;			 
				LoadNode(rootNode, meshGroup, fbxSkeleton, materialMap);

				if (hasSkeleton)
				{
					StuffSkletonData(fbxSkeleton, joint);
					CheckModelAxis(meshGroup, joint);
					result = (J_FBX_RESULT)((int)result | (int)J_FBX_RESULT_HAS_SKELETON);
				}
				fbxSkeleton.joint.clear(); 
				scene->Destroy();
				importer->Destroy();
				dataSet = nullptr;
				return result;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return J_FBX_RESULT_FAIL;
			}
		}
		J_FBX_RESULT JFbxFileLoader::LoadFbxAnimationFile(const std::string& path, JFbxAnimationData& jfbxAniData)
		{
			if (fbxManager == nullptr)
			{
				fbxManager = FbxManager::Create();
				FbxIOSettings* fbxIoSetting = FbxIOSettings::Create(fbxManager, IOSROOT);
				fbxManager->SetIOSettings(fbxIoSetting);
			}
			FbxImporter* importer = FbxImporter::Create(fbxManager, "");
			bool status = importer->Initialize(path.c_str(), -1, fbxManager->GetIOSettings());
			if (!status)
			{
				printf("Call to Fbx::Initialize() failed.\n");
				printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
				return J_FBX_RESULT_FAIL;
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");

			//fbx file 내용을 scene으로 가져온다
			if (importer->Import(scene))
			{
				dataSet = std::make_unique<FbxLoadDataSet>();

				FbxNode* rootNode = scene->GetRootNode();
				SetSceneAxis(scene, rootNode);

				FbxGeometryConverter geometryConverter(fbxManager);
				geometryConverter.Triangulate(scene, true);
				J_FBX_RESULT result;
				 
				//hasRootJoint = true;
				 
				JFbxSkeleton fbxSkeleton;
				LoadJoint(rootNode, -1, -1, -1, fbxSkeleton);

				//ProcessSkeletonHierarchy(rootNode, fbxSkeleton);
				//fbxSkeleton.joint[0].parentIndex == 0 add emptyNode in front

				bool hasSkeleton = (fbxSkeleton.jointCount > 0);
				result = LoadAnimationClip(scene, rootNode, fbxSkeleton, hasSkeleton, jfbxAniData);

				if (hasSkeleton)
				{ 
					result = (J_FBX_RESULT)((int)result | (int)J_FBX_RESULT_HAS_SKELETON);
					std::wstring totalName;
					for (uint i = 0; i < fbxSkeleton.jointCount; ++i)
						totalName += JCUtil::U8StrToWstr(fbxSkeleton.joint[i].name);
					jfbxAniData.skeletonHash = JCUtil::CalculateGuid(totalName);
				}
				fbxSkeleton.joint.clear(); 
				scene->Destroy();
				importer->Destroy();
				dataSet = nullptr;
				return result;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return J_FBX_RESULT_FAIL;
			}
		}
		JFbxFileLoader::FbxFileTypeInfo JFbxFileLoader::GetFileTypeInfo(const std::string& path)
		{
			if (fbxManager == nullptr)
			{
				fbxManager = FbxManager::Create();
				FbxIOSettings* fbxIoSetting = FbxIOSettings::Create(fbxManager, IOSROOT);
				fbxManager->SetIOSettings(fbxIoSetting);
			}
			FbxImporter* importer = FbxImporter::Create(fbxManager, "");
			bool status = importer->Initialize(path.c_str(), -1, fbxManager->GetIOSettings());
			if (!status)
			{
				printf("Call to Fbx::Initialize() failed.\n");
				printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
				return FbxFileTypeInfo();
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");
			if (importer->Import(scene))
			{
				FbxFileTypeInfo res;
				res.meshCount = 0;
				res.jointCount = 0;
				res.materialCount = scene->GetMaterialCount();
				GetMeshCount(scene->GetRootNode(), res.meshCount, res);
				GetJointCount(scene->GetRootNode(), res.jointCount, res);
				 
				FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();
				if (animStack != nullptr && res.jointCount > 0)
				{
					res.animName = animStack->GetName();
					res.typeInfo = J_FBX_RESULT_HAS_ANIMATION;
				}

				if (res.meshCount > 0)
					res.typeInfo = Core::AddSQValueEnum(res.typeInfo, J_FBX_RESULT_HAS_MESH);
				if (res.jointCount > 0)
					res.typeInfo = Core::AddSQValueEnum(res.typeInfo, J_FBX_RESULT_HAS_SKELETON);  
				scene->Destroy();
				importer->Destroy();
				return res;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return FbxFileTypeInfo();
			}
		}
		void JFbxFileLoader::GetMeshCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo)
		{
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
			if (nodeAttribute)
			{
				if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					++count;
					if (typeInfo.meshRootName.empty())
						typeInfo.meshRootName = node->GetName();
				}
			}

			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				GetMeshCount(node->GetChild(i), count, typeInfo);
		}
		void JFbxFileLoader::GetJointCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo)
		{
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
			if (nodeAttribute && nodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
			{
				++count;
				if (typeInfo.skeletonRootName.empty())
					typeInfo.skeletonRootName = node->GetName();
			}

			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				GetJointCount(node->GetChild(i), count, typeInfo);
		}
		void JFbxFileLoader::LoadNode(FbxNode* node, JStaticMeshGroup& meshGroup, JFbxMaterialMap& materialMap)
		{
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();	 
			if (nodeAttribute)
			{
				if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					LoadControlPoint(node->GetMesh()); 
					SortBlendingWeight();
					std::unique_ptr<JStaticMeshData> staticMesh = LoadStaticMesh(node);
					if (staticMesh->IsValid())
					{
						StuffMaterial(node, materialMap, staticMesh->GetGuid());
						staticMesh->InverseIndex();
						meshGroup.AddMeshData(std::move(staticMesh));				 
					} 
				}
			}
			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				LoadNode(node->GetChild(i), meshGroup, materialMap);
		}
		void JFbxFileLoader::LoadNode(FbxNode* node, JSkinnedMeshGroup& meshGroup, JFbxSkeleton& skeleton, JFbxMaterialMap& materialMap)
		{
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
			if (nodeAttribute)
			{
				if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					LoadControlPoint(node->GetMesh());
					LoadSkinnedMeshInfo(node, skeleton);
					SortBlendingWeight();
					  
					std::unique_ptr<JSkinnedMeshData> skinnedMesh = LoadSkinnedMesh(node, skeleton);
					if (skinnedMesh->IsValid())
					{
						StuffMaterial(node, materialMap, skinnedMesh->GetGuid());
						skinnedMesh->InverseIndex();
						meshGroup.AddMeshData(std::move(skinnedMesh));				
					} 
				}
			}
			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				LoadNode(node->GetChild(i), meshGroup, skeleton, materialMap);
		}
		void JFbxFileLoader::LoadJoint(FbxNode* node, int depth, int index, int parentIndex, JFbxSkeleton& skeleton)
		{
			//stream << JCUtil::StrToWstr(node->GetName()) << '\n';
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();

			if (nodeAttribute && nodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
			{
				if (parentIndex == 0 && skeleton.joint.size() == 0)
				{ 
					JFbxJoint rootjoint;
					rootjoint.parentIndex = -1;
					rootjoint.name = node->GetParent()->GetName();
					rootjoint.node = node->GetParent();
					rootjoint.max = JVector3<float>::NegativeInfV();
					rootjoint.min = JVector3<float>::PositiveInfV();
					skeleton.joint.push_back(rootjoint);
					++skeleton.jointCount;
					++index;
					//hasRootJoint = false;
				} 
				JFbxJoint joint;
				joint.parentIndex = parentIndex;
				joint.name = node->GetName();
				joint.node = node;
				joint.max = JVector3<float>::NegativeInfV();
				joint.min = JVector3<float>::PositiveInfV();
				joint.size = (float)node->GetSkeleton()->Size;

				skeleton.joint.push_back(joint);
				++skeleton.jointCount;
				//stream << L"Name: " << JCUtil::StrToWstr(joint.name) << '\n';
				///stream << L"Index: " << index << '\n';
				//stream << L"Parent: " << parentIndex << '\n';
				//stream << '\n';
			}

			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				LoadJoint(node->GetChild(i), depth + 1, (int)skeleton.joint.size(), index, skeleton);
		}
		std::unique_ptr<JStaticMeshData> JFbxFileLoader::LoadStaticMesh(FbxNode* node)
		{
			FbxMesh* mesh = node->GetMesh();
			if (!mesh->GetNode())
				return std::make_unique<JStaticMeshData>();
 
			std::vector<JStaticMeshVertex> vertices;
			std::vector<uint32> indices;

			uint triangleCount = mesh->GetPolygonCount();
			uint idx[3];
			dataSet->vertexCount = 0;
			dataSet->vertexIndexMap.clear();

			DirectX::XMFLOAT3 positionXm[3];
			DirectX::XMFLOAT2 textureXm[3];
			DirectX::XMFLOAT3 normalXm[3];
			DirectX::XMFLOAT3 biNormalXm[3];
			DirectX::XMFLOAT4 tangentXm[3];
			for (uint i = 0; i < triangleCount; ++i)
			{
				bool hasNormal;
				bool hasUv;
				bool hasBinormal;
				bool hasTangent;
				for (uint j = 0; j < 3; ++j)
				{
					uint index = dataSet->indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);
					idx[j] = index;
					positionXm[j] = dataSet->controlPoint[controlPointIndex].position.ToSimilar<XMFLOAT3>();

					hasUv = LoadTextureUV(mesh, controlPointIndex, mesh->GetTextureUVIndex(i, index), textureXm[j]);
					hasNormal = LoadNormal(mesh, controlPointIndex, dataSet->vertexCount, normalXm[j]);
					hasBinormal = LoadBinormal(mesh, controlPointIndex, dataSet->vertexCount, biNormalXm[j]);
					hasTangent = LoadTangent(mesh, controlPointIndex, dataSet->vertexCount, tangentXm[j]);
					textureXm[j].y = 1 - textureXm[j].y;
					++dataSet->vertexCount;
				}

				if (!hasTangent)
				{
					ComputeTangentFrame(idx, 1, positionXm, normalXm, textureXm, 3, tangentXm);
				}
				 
				for (uint j = 0; j < 3; ++j)
				{
					uint index = dataSet->indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);
					  
					JStaticMeshVertex newVertex(positionXm[j], normalXm[j], textureXm[j], tangentXm[j]);
					size_t guid = MakeVertexMapKey(positionXm[j], textureXm[j], normalXm[j], biNormalXm[j], tangentXm[j], dataSet->controlPoint[controlPointIndex].isSkin);
				
					auto mapData = dataSet->vertexIndexMap.find(guid);
					if (mapData != dataSet->vertexIndexMap.end())
						indices.push_back(mapData->second);
					else
					{
						uint vertexIndex = (uint)vertices.size();
						dataSet->vertexIndexMap.emplace(guid, vertexIndex);
						indices.push_back(vertexIndex);
						vertices.push_back(newVertex);
					}
				}
			}
			return std::make_unique<JStaticMeshData>(JCUtil::StrToWstr(node->GetName()), MakeGuid(), std::move(indices), true, true, std::move(vertices));
		}
		std::unique_ptr<JSkinnedMeshData> JFbxFileLoader::LoadSkinnedMesh(FbxNode* node, JFbxSkeleton& skeleton)
		{
			FbxMesh* mesh = node->GetMesh();
			if (!mesh->GetNode())
				return std::make_unique<JSkinnedMeshData>(); 

			std::vector<JSkinnedMeshVertex> vertices;
			std::vector<uint32> indices;

			uint triangleCount = mesh->GetPolygonCount();
			uint idx[3];
			dataSet->vertexCount = 0;
			dataSet->vertexIndexMap.clear();

			DirectX::XMFLOAT3 positionXm[3];
			DirectX::XMFLOAT2 textureXm[3];
			DirectX::XMFLOAT3 normalXm[3];
			DirectX::XMFLOAT3 biNormalXm[3];
			DirectX::XMFLOAT4 tangentXm[3];
			for (uint i = 0; i < triangleCount; ++i)
			{
				bool hasNormal = false;
				bool hasUv = false;
				bool hasBinormal = false;
				bool hasTangent = false;
				for (uint j = 0; j < 3; ++j)
				{
					uint index = dataSet->indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);
					idx[j] = index;
					positionXm[j] = dataSet->controlPoint[controlPointIndex].position.ToSimilar<XMFLOAT3>();

					hasUv = LoadTextureUV(mesh, controlPointIndex, mesh->GetTextureUVIndex(i, index), textureXm[j]);
					hasNormal = LoadNormal(mesh, controlPointIndex, dataSet->vertexCount, normalXm[j]);
					hasBinormal = LoadBinormal(mesh, controlPointIndex, dataSet->vertexCount, biNormalXm[j]);
					hasTangent = LoadTangent(mesh, controlPointIndex, dataSet->vertexCount, tangentXm[j]);

					textureXm[j].y = 1 - textureXm[j].y;
					++dataSet->vertexCount;
				}

				if (!hasTangent)
					ComputeTangentFrame(idx, 1, positionXm, normalXm, textureXm, 3, tangentXm);

				for (uint j = 0; j < 3; ++j)
				{
					uint index = dataSet->indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);

					size_t guid = MakeVertexMapKey(positionXm[j], textureXm[j], normalXm[j], biNormalXm[j], tangentXm[j], dataSet->controlPoint[controlPointIndex].isSkin);
					auto data = dataSet->vertexIndexMap.find(guid);
					
					if (data != dataSet->vertexIndexMap.end())
						indices.push_back(data->second);
					else
					{   
						JSkinnedMeshVertex newVertex(positionXm[j], normalXm[j], textureXm[j], tangentXm[j], dataSet->controlPoint[controlPointIndex].blendingInfo);
						uint32 vertexIndex = (uint32)vertices.size();
						dataSet->vertexIndexMap.emplace(guid, vertexIndex);

						indices.push_back(vertexIndex);
						vertices.push_back(newVertex);

						JVector3 pos = JVector3<float>(positionXm[j].x, positionXm[j].y, positionXm[j].z);
						for (int k = 0; k < dataSet->controlPoint[controlPointIndex].blendingInfo.size(); ++k)
						{
							int jointIndex = dataSet->controlPoint[controlPointIndex].blendingInfo[k].blendingIndex;
							if (dataSet->controlPoint[controlPointIndex].blendingInfo[k].blendingWeight > 0.5f)
							{
								if (skeleton.joint[jointIndex].min > pos)
									skeleton.joint[jointIndex].min = pos;
								if (skeleton.joint[jointIndex].max < pos)
									skeleton.joint[jointIndex].max = pos;
							}
						}
					}
				}
			}

			return  std::make_unique<JSkinnedMeshData>(JCUtil::StrToWstr(node->GetName()), MakeGuid(), std::move(indices), true, true, std::move(vertices));
		}
		void JFbxFileLoader::LoadControlPoint(FbxMesh* mesh)
		{  
			uint count = mesh->GetControlPointsCount();

			dataSet->controlPoint.clear();
			dataSet->controlPoint.resize(count);
			FbxVector4 trans;
			for (uint i = 0; i < count; ++i)
			{
				trans = mesh->GetControlPointAt(i);
				SetLeftHandPosition(trans);
				dataSet->controlPoint[i].position.x = static_cast<float>(trans[0]);
				dataSet->controlPoint[i].position.y = static_cast<float>(trans[1]);
				dataSet->controlPoint[i].position.z = static_cast<float>(trans[2]);
				ResizeVertexPosition(dataSet->controlPoint[i].position);
			}
		}
		bool JFbxFileLoader::LoadTextureUV(const FbxMesh* mesh, int controlPointIndex, int inTextureUVIndex, XMFLOAT2& outUV)
		{
			outUV = XMFLOAT2(0, 0);
			if (mesh->GetElementUVCount() < 1)
				return false;

			const FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);

			switch (vertexUV->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[0]);
					outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[1]);
				}
				break;

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexUV->GetIndexArray().GetAt(controlPointIndex);
					outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
					outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
				}
				break;
				default:
					throw std::exception("Invalid Reference");
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				case FbxGeometryElement::eIndexToDirect:
				{
					outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
					outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
				}
				break;
				default: throw std::exception("Invalid Reference");
				}

				break;
			default:
				break;
			}
			return true;
		}
		bool JFbxFileLoader::LoadNormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, XMFLOAT3& outNormal)
		{
			outNormal = XMFLOAT3(0, 0, 0);
			if (mesh->GetElementNormalCount() < 1)
				return false;

			const FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);

			switch (vertexNormal->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexNormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					FbxVector4 normal = vertexNormal->GetDirectArray().GetAt(controlPointIndex);
					SetLeftHandPosition(normal);
					outNormal.x = static_cast<float>(normal.mData[0]);
					outNormal.y = static_cast<float>(normal.mData[1]);
					outNormal.z = static_cast<float>(normal.mData[2]);
				}
				break;

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);
					FbxVector4 normal = vertexNormal->GetDirectArray().GetAt(index);
					SetLeftHandPosition(normal);
					outNormal.x = static_cast<float>(normal.mData[0]);
					outNormal.y = static_cast<float>(normal.mData[1]);
					outNormal.z = static_cast<float>(normal.mData[2]);
				}
				break;

				default:
					break;
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexNormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					FbxVector4 normal = vertexNormal->GetDirectArray().GetAt(vertexCounter);
					SetLeftHandPosition(normal);
					outNormal.x = static_cast<float>(normal.mData[0]);
					outNormal.y = static_cast<float>(normal.mData[1]);
					outNormal.z = static_cast<float>(normal.mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexNormal->GetIndexArray().GetAt(vertexCounter);
					FbxVector4 normal = vertexNormal->GetDirectArray().GetAt(index);
					SetLeftHandPosition(normal);
					outNormal.x = static_cast<float>(normal.mData[0]);
					outNormal.y = static_cast<float>(normal.mData[1]);
					outNormal.z = static_cast<float>(normal.mData[2]);
				}
				break;
				default:
					break;
				}
				break;
			}

			return true;
		}
		bool JFbxFileLoader::LoadBinormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, XMFLOAT3& outBinormal)
		{
			outBinormal = XMFLOAT3(0, 0, 0);
			if (mesh->GetElementBinormalCount() < 1)
				return false;

			const FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal(0);

			switch (vertexBinormal->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexBinormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					FbxVector4 biNormal = vertexBinormal->GetDirectArray().GetAt(controlPointIndex);
					SetLeftHandPosition(biNormal);
					outBinormal.x = static_cast<float>(biNormal.mData[0]);
					outBinormal.y = static_cast<float>(biNormal.mData[1]);
					outBinormal.z = static_cast<float>(biNormal.mData[2]);
				}
				break;

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexBinormal->GetIndexArray().GetAt(controlPointIndex);
					FbxVector4 biNormal = vertexBinormal->GetDirectArray().GetAt(index);
					SetLeftHandPosition(biNormal);
					outBinormal.x = static_cast<float>(biNormal.mData[0]);
					outBinormal.y = static_cast<float>(biNormal.mData[1]);
					outBinormal.z = static_cast<float>(biNormal.mData[2]);
				}
				break;
				default:
					throw std::exception("Invalid Reference");
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexBinormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					FbxVector4 biNormal = vertexBinormal->GetDirectArray().GetAt(vertexCounter);
					SetLeftHandPosition(biNormal);
					outBinormal.x = static_cast<float>(biNormal.mData[0]);
					outBinormal.y = static_cast<float>(biNormal.mData[1]);
					outBinormal.z = static_cast<float>(biNormal.mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexBinormal->GetIndexArray().GetAt(vertexCounter);
					FbxVector4 biNormal = vertexBinormal->GetDirectArray().GetAt(index);
					SetLeftHandPosition(biNormal);
					outBinormal.x = static_cast<float>(biNormal.mData[0]);
					outBinormal.y = static_cast<float>(biNormal.mData[1]);
					outBinormal.z = static_cast<float>(biNormal.mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
				}

				break;
			default:
				break;
			}
			return true;
		}
		bool JFbxFileLoader::LoadTangent(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, XMFLOAT4& outTangent)
		{
			outTangent = XMFLOAT4(0, 0, 0, 0);
			if (mesh->GetElementTangentCount() < 1)
				return false;

			const FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent(0);

			switch (vertexTangent->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (vertexTangent->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					FbxVector4 tangent = vertexTangent->GetDirectArray().GetAt(controlPointIndex);
					SetLeftHandPosition(tangent);
					outTangent.x = static_cast<float>(tangent.mData[0]);
					outTangent.y = static_cast<float>(tangent.mData[1]);
					outTangent.z = static_cast<float>(tangent.mData[2]);
				}
				break;

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexTangent->GetIndexArray().GetAt(controlPointIndex);
					FbxVector4 tangent = vertexTangent->GetDirectArray().GetAt(index);
					SetLeftHandPosition(tangent);
					outTangent.x = static_cast<float>(tangent.mData[0]);
					outTangent.y = static_cast<float>(tangent.mData[1]);
					outTangent.z = static_cast<float>(tangent.mData[2]);
				}
				break;
				default:
					throw std::exception("Invalid Reference");
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (vertexTangent->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					FbxVector4 tangent = vertexTangent->GetDirectArray().GetAt(vertexCounter);
					SetLeftHandPosition(tangent);
					outTangent.x = static_cast<float>(tangent.mData[0]);
					outTangent.y = static_cast<float>(tangent.mData[1]);
					outTangent.z = static_cast<float>(tangent.mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexTangent->GetIndexArray().GetAt(vertexCounter);
					FbxVector4 tangent = vertexTangent->GetDirectArray().GetAt(index);
					SetLeftHandPosition(tangent);

					outTangent.x = static_cast<float>(tangent.mData[0]);
					outTangent.y = static_cast<float>(tangent.mData[1]);
					outTangent.z = static_cast<float>(tangent.mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
				}

				break;
			default:
				break;
			}
			return true;
		}

		//Deformer: cluster를 포함
		//Cluster: link 포함
		//link
		void JFbxFileLoader::LoadSkinnedMeshInfo(FbxNode* node, JFbxSkeleton& skeleton)
		{
			FbxMesh* currMesh = node->GetMesh();
			uint numOfDeformers = currMesh->GetDeformerCount();
			FbxAMatrix geometryTransform = GetGeometryTransformation(node);

			for (uint i = 0; i < numOfDeformers; ++i)
			{
				FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(i, FbxDeformer::eSkin));

				if (!currSkin)
					continue;

				uint clusterCount = currSkin->GetClusterCount();
				for (uint j = 0; j < clusterCount; ++j)
				{
					FbxCluster* cluster = currSkin->GetCluster(j);

					FbxAMatrix transformMatrix;
					FbxAMatrix transformLinkMatrix;
					FbxAMatrix globalBindposeInverseMatrix;

					int jointIndex = -1;

					std::string jointName = cluster->GetLink()->GetName();
					for (uint k = 0; k < skeleton.jointCount; ++k)
						if (skeleton.joint[k].name == jointName)
						{
							jointIndex = (int)k;
							break;
						}
					if (jointIndex == -1)
						continue;

					cluster->GetTransformMatrix(transformMatrix);
					cluster->GetTransformLinkMatrix(transformLinkMatrix);
					skeleton.joint[jointIndex].globalBindPositionInverse = (transformLinkMatrix.Inverse() * transformMatrix * geometryTransform);
					FbxAMatrix bind = skeleton.joint[jointIndex].globalBindPositionInverse.Inverse();
					SetLeftHandMatrix(bind);
					skeleton.joint[jointIndex].globalBindPositionInverse = bind.Inverse();

					int* pCtrlPtIdx = cluster->GetControlPointIndices();
					uint IndicesCount = cluster->GetControlPointIndicesCount();
					double* boneWeights = cluster->GetControlPointWeights();

					for (uint k = 0; k < IndicesCount; ++k)
					{
						JBlendingIndexWeightPair blend;
						blend.blendingIndex = (BYTE)jointIndex;
						blend.blendingWeight = (float)boneWeights[k];
						dataSet->controlPoint[pCtrlPtIdx[k]].blendingInfo.push_back(blend);
						dataSet->controlPoint[pCtrlPtIdx[k]].isSkin = true;
					}
				}
			}
			JBlendingIndexWeightPair blendingIndexWeightPair;
			blendingIndexWeightPair.blendingIndex = 0;
			blendingIndexWeightPair.blendingWeight = 0;
			uint controlPointSize = (uint)dataSet->controlPoint.size();
			for (uint i = 0; i < controlPointSize; ++i)
			{
				for (uint j = (uint)dataSet->controlPoint[i].blendingInfo.size(); j < 4; ++j)
				{
					dataSet->controlPoint[i].blendingInfo.push_back(blendingIndexWeightPair);
				}
			}
		}
		J_FBX_RESULT JFbxFileLoader::LoadAnimationClip(FbxScene* scene, FbxNode* node, JFbxSkeleton& skeleton, bool hasSkeleton, JFbxAnimationData& fbxAniData)
		{
			FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();
			if (!animStack)
				return J_FBX_RESULT_FAIL;

			int animStackcnt = scene->GetSrcObjectCount< FbxAnimStack>();
			FbxString animStackName = animStack->GetName();
			FbxTakeInfo* takeInfo = scene->GetTakeInfo(animStackName);
			FbxTime animationStart = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime animationEnd = takeInfo->mLocalTimeSpan.GetStop();
			size_t animationLength = animationEnd.GetFrameCount(FbxTime::eFrames30) - animationStart.GetFrameCount(FbxTime::eFrames30) + 1;
			FbxAMatrix geometryTransform = GetGeometryTransformation(node);
			FbxAMatrix InvBoneInitPoseGlobalTransform;
			FbxAMatrix matBeforeMatrix;
			FbxAMatrix matNowMatrix;
			FbxAMatrix currentTransformOffset;
			JMatrix4x4 mat;

			fbxAniData.length = (uint)animationLength;
			fbxAniData.framePerSecond = 30;
			fbxAniData.animationSample.resize(skeleton.jointCount);

			for (uint i = 0; i < skeleton.jointCount; ++i)
			{
				std::string pName = skeleton.joint[i].name;
				matBeforeMatrix.SetIdentity();

				for (FbxLongLong j = animationStart.GetFrameCount(FbxTime::eFrames30);
					j <= animationEnd.GetFrameCount(FbxTime::eFrames30); ++j)
				{
					FbxTime currTime;
					currTime.SetFrame(j, FbxTime::eFrames30);

					//InvBoneInitPoseGlobalTransform = skeleton.joint[0].node->EvaluateGlobalTransform(currTime) *
					//	geometryTransform; 
					matNowMatrix = scene->GetAnimationEvaluator()->GetNodeGlobalTransform(skeleton.joint[i].node, currTime) * geometryTransform;
					SetLeftHandMatrix(matNowMatrix);
					if (matNowMatrix == matBeforeMatrix)
						continue;

					matBeforeMatrix = matNowMatrix;
					ConvertFbaToJM(matNowMatrix, mat);
					ResizeMatrix(mat);
					fbxAniData.animationSample[i].jointPose.emplace_back(mat, (float)currTime.GetSecondDouble());
				}
			}
			return J_FBX_RESULT_HAS_ANIMATION;
		}
		void JFbxFileLoader::LoadMaterial(FbxScene* scene)
		{ 
			if (scene == nullptr)
				return;
 
			uint matCount = scene->GetMaterialCount();
			for (uint i = 0; i < matCount; ++i)
			{
				FbxSurfaceMaterial* fbxMaterial = scene->GetMaterial(i);
				if (fbxMaterial == nullptr)
					continue;

				std::string name = fbxMaterial->GetName(); 
				if (dataSet->mateiralMap.find(name) != dataSet->mateiralMap.end())
					continue;
				 
				JFbxMaterial newMaterial;
				newMaterial.name = name;
				if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
				{
					FbxSurfaceLambert* lam = FbxCast< FbxSurfaceLambert>(fbxMaterial);
					newMaterial.mParam.albedoColor = JVector4F(Private::Convert(lam->Diffuse), 1.0f);
					//Develop::JDevelopDebug::PushLog("Lambert Material: " + name);
				}
				else if (fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
				{
					FbxSurfacePhong* phong = FbxCast<FbxSurfacePhong>(fbxMaterial);
					newMaterial.mParam.albedoColor = JVector4F(Private::Convert(phong->Diffuse), phong->DiffuseFactor);
					newMaterial.mParam.roughness = std::clamp((float)(1.0f - phong->Shininess.Get()), 0.0f, 1.0f);
					//Develop::JDevelopDebug::PushLog("Phong Material: " + name);
				}
				else
				{ 
					//Develop::JDevelopDebug::PushLog("Other Material: " + name);
				} 
				FbxProperty emissive = fbxMaterial->FindProperty(fbxMaterial->sEmissive);
				FbxProperty emissiveFactor = fbxMaterial->FindProperty(fbxMaterial->sEmissiveFactor);
				FbxProperty ambient = fbxMaterial->FindProperty(fbxMaterial->sAmbient);
				FbxProperty ambientFactor = fbxMaterial->FindProperty(fbxMaterial->sAmbientFactor);
				FbxProperty diffuse = fbxMaterial->FindProperty(fbxMaterial->sDiffuse);
				FbxProperty diffuseFactor = fbxMaterial->FindProperty(fbxMaterial->sDiffuseFactor);
				
				FbxProperty specular = fbxMaterial->FindProperty(fbxMaterial->sSpecular);
				FbxProperty specularFactor = fbxMaterial->FindProperty(fbxMaterial->sSpecularFactor);
				FbxProperty shininess = fbxMaterial->FindProperty(fbxMaterial->sShininess);
				FbxProperty bump = fbxMaterial->FindProperty(fbxMaterial->sBump);
				FbxProperty normalMap = fbxMaterial->FindProperty(fbxMaterial->sNormalMap);
				FbxProperty bumpFactor = fbxMaterial->FindProperty(fbxMaterial->sBumpFactor);

				FbxProperty transparentColor = fbxMaterial->FindProperty(fbxMaterial->sTransparentColor);
				FbxProperty transparencyFactor = fbxMaterial->FindProperty(fbxMaterial->sTransparencyFactor);
				FbxProperty reflection = fbxMaterial->FindProperty(fbxMaterial->sReflection);
				FbxProperty reflectionFactor = fbxMaterial->FindProperty(fbxMaterial->sReflectionFactor);
				FbxProperty displacementColor = fbxMaterial->FindProperty(fbxMaterial->sDisplacementColor);
				FbxProperty displacementFactor = fbxMaterial->FindProperty(fbxMaterial->sDisplacementFactor);
				FbxProperty vectorDisplacementColor = fbxMaterial->FindProperty(fbxMaterial->sVectorDisplacementColor);
				FbxProperty vectorDisplacementFactor = fbxMaterial->FindProperty(fbxMaterial->sVectorDisplacementFactor);

				std::string emissiveName;
				std::string emissiveFactorName;
				std::string ambientName;
				std::string ambientFactorName;
				std::string diffuseName;
				std::string diffuseFactorName;

				std::string specularName;
				std::string specularFactorName;
				std::string shininessName;
				std::string bumpName;
				std::string normalMapName;
				std::string bumpFactorName;

				std::string transparentColorName;
				std::string transparencyFactorName;
				std::string reflectionName;
				std::string reflectionFactorName;
				std::string displacementColorName;
				std::string displacementFactorName;
				std::string vectorDisplacementColorName;
				std::string vectorDisplacementFactorName;

				LoadTexture(emissive, emissiveName);
				LoadTexture(emissiveFactor, emissiveFactorName);
				LoadTexture(ambient, ambientName);
				LoadTexture(ambientFactor, ambientFactorName);
				LoadTexture(diffuse, diffuseName);
				LoadTexture(diffuseFactor, diffuseFactorName);
				LoadTexture(specular, specularName);
				LoadTexture(specularFactor, specularFactorName);
				LoadTexture(shininess, shininessName);
				LoadTexture(bump, bumpName);
				LoadTexture(normalMap, normalMapName);
				LoadTexture(bumpFactor, bumpFactorName);
				LoadTexture(transparentColor, transparentColorName);
				LoadTexture(transparencyFactor, transparencyFactorName);
				LoadTexture(reflection, reflectionName);
				LoadTexture(reflectionFactor, reflectionFactorName);
				LoadTexture(displacementColor, displacementColorName);
				LoadTexture(displacementFactor, displacementFactorName);
				LoadTexture(vectorDisplacementColor, vectorDisplacementColorName);
				LoadTexture(vectorDisplacementFactor, vectorDisplacementFactorName);

				newMaterial.albedoMapName = diffuseName;
				newMaterial.normalMapName = bumpName.empty() ? normalMapName : bumpName;
				newMaterial.heightMapName = displacementColorName;
				newMaterial.metalicMapName = reflectionName;
				newMaterial.roughnessMapName = shininessName;
				newMaterial.ambientMapName = ambientName;
				newMaterial.specularName = specularName; 

				//shininess, specularFactor등 값들의 정확한 범위를 파악하고나서 대입 코드추가할것.
				//newMaterial.roughness = 1.0f - shininess;
				//newMaterial.specularFactor;

				/*			
				Develop::JDevelopDebug::PushLog("emissive: " + emissiveName);
				Develop::JDevelopDebug::PushLog("emissiveFactor: " + emissiveFactorName);
				Develop::JDevelopDebug::PushLog("ambient: " + ambientName);
				Develop::JDevelopDebug::PushLog("ambientFactor: " + ambientFactorName);
				Develop::JDevelopDebug::PushLog("diffuse: " + diffuseName);
				Develop::JDevelopDebug::PushLog("diffuseFactor: " + diffuseFactorName);
				Develop::JDevelopDebug::PushLog("specular: " + specularName);
				Develop::JDevelopDebug::PushLog("specularFactor: " + specularFactorName);
				Develop::JDevelopDebug::PushLog("shininess: " + shininessName);
				Develop::JDevelopDebug::PushLog("bump: " + bumpName);
				Develop::JDevelopDebug::PushLog("normalMap: " + normalMapName);
				Develop::JDevelopDebug::PushLog("bumpFactor: " + bumpFactorName);
				Develop::JDevelopDebug::PushLog("transparentColor: " + transparentColorName);
				Develop::JDevelopDebug::PushLog("transparencyFactor: " + transparencyFactorName);
				Develop::JDevelopDebug::PushLog("reflection: " + reflectionName);
				Develop::JDevelopDebug::PushLog("reflectionFactor: " + reflectionFactorName);
				Develop::JDevelopDebug::PushLog("displacementColor: " + displacementColorName);
				Develop::JDevelopDebug::PushLog("displacementFactor: " + displacementFactorName);
				Develop::JDevelopDebug::PushLog("vectorDisplacementColor: " + vectorDisplacementColorName);
				Develop::JDevelopDebug::PushLog("vectorDisplacementFactor: " + vectorDisplacementFactorName);
				*/
				/*
				LoadTexture(diffuse, newMaterial.albedoMapName);
				LoadTexture(normal, newMaterial.normalMapName);
				LoadTexture(bump, newMaterial.bumpMapName);
				LoadTexture(ambient, newMaterial.ambientMapName);
				LoadTexture(displacementColor, newMaterial.heightMapName);
				LoadTexture(shiness, newMaterial.roughnessMapName);
				LoadTexture(reflection, newMaterial.metalicMapName);
			 
				LoadTexture(emissive, newMaterial.emissiveName);
				LoadTexture(specular, newMaterial.specularName);
				LoadTexture(transparentColor, newMaterial.transparentColorName);
				LoadTexture(vectorDisplacementColor, newMaterial.vectorDisplacementColorName); 

				Develop::JDevelopDebug::PushLog("Albedo: " + newMaterial.albedoColor.ToString());
				Develop::JDevelopDebug::PushLog("Roughness: " + std::to_string(newMaterial.roughness));
				Develop::JDevelopDebug::PushLog("Metalic: " + std::to_string(newMaterial.metallic));
				Develop::JDevelopDebug::PushLog("AlbedoMap: " + newMaterial.albedoMapName);
				Develop::JDevelopDebug::PushLog("NormalMap: " + newMaterial.normalMapName);
				Develop::JDevelopDebug::PushLog("BumpMapName: " + newMaterial.bumpMapName);
				Develop::JDevelopDebug::PushLog("AmbientMap: " + newMaterial.ambientMapName);
				Develop::JDevelopDebug::PushLog("HeightMap: " + newMaterial.heightMapName);
				Develop::JDevelopDebug::PushLog("RoughnessMapName: " + newMaterial.roughnessMapName);
				Develop::JDevelopDebug::PushLog("MetalicMapName: " + newMaterial.metalicMapName);

				Develop::JDevelopDebug::PushLog("EmissiveName: " + newMaterial.emissiveName);
				Develop::JDevelopDebug::PushLog("SpecularName: " + newMaterial.specularName);
				Develop::JDevelopDebug::PushLog("TransparentColorName: " + newMaterial.transparentColorName);
				Develop::JDevelopDebug::PushLog("VectorDisplacementColorName: " + newMaterial.vectorDisplacementColorName);
				*/

				dataSet->mateiralMap.emplace(name, newMaterial);
			}
		} 
		bool JFbxFileLoader::LoadTexture(FbxProperty& prop, std::string& path)
		{
			path = "";
			const uint layerTextureCount = prop.GetSrcObjectCount(FbxCriteria::ObjectType(FbxLayeredTexture::ClassId));
			if (layerTextureCount != 0)
			{
				FbxLayeredTexture* layeredTexture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject(FbxCriteria::ObjectType(FbxLayeredTexture::ClassId), 0));
				FbxFileTexture* texture = FbxCast<FbxFileTexture>(layeredTexture->GetSrcObject<FbxTexture>(0));
				path = texture->GetFileName();
				return true;
			}
			const uint textureCount = prop.GetSrcObjectCount(FbxCriteria::ObjectType(FbxTexture::ClassId));
			if (textureCount == 0)
				return false; 
			 ;
			FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject(FbxCriteria::ObjectType(FbxTexture::ClassId), 0));
			path = texture->GetFileName();
			return true;
		}
		void JFbxFileLoader::SortBlendingWeight()
		{
			uint count = (uint)dataSet->controlPoint.size();
			for (uint i = 0; i < count; ++i)
			{
				sort(dataSet->controlPoint[i].blendingInfo.begin(), dataSet->controlPoint[i].blendingInfo.end(), CompareBlendingWeight);
			}
		}
		void JFbxFileLoader::StuffSkletonData(JFbxSkeleton& fbxSkeleton, std::vector<Joint>& joint)
		{   
			joint.resize(fbxSkeleton.jointCount);
			 
			XMVECTOR skeletonMaxXmV = JVector3<float>::NegativeInfV().ToXmV();
			XMVECTOR skeletonMinXmV = JVector3<float>::PositiveInfV().ToXmV();

			for (uint i = 0; i < fbxSkeleton.jointCount; ++i)
			{
				joint[i].name = JCUtil::StrToWstr(fbxSkeleton.joint[i].name);
				joint[i].parentIndex = fbxSkeleton.joint[i].parentIndex;
				ConvertFbaToJM(fbxSkeleton.joint[i].globalBindPositionInverse, joint[i].inbindPose);
				ResizeMatrix(joint[i].inbindPose);

				if (i > 0)
				{
					const uint8 parentIndex = joint[i].parentIndex;
					const XMMATRIX parentWorldBind = XMMatrixInverse(nullptr, joint[parentIndex].inbindPose.LoadXM());
					const XMMATRIX childWorldBind = XMMatrixInverse(nullptr, joint[i].inbindPose.LoadXM());
					const XMMATRIX childLocalBind = XMMatrixMultiply(childWorldBind, XMMatrixInverse(nullptr, parentWorldBind));

					XMVECTOR childWorldS;
					XMVECTOR childWorldQ;
					XMVECTOR childWorldT;
					XMVECTOR childLocalS;
					XMVECTOR childLocalQ;
					XMVECTOR childLocalT;

					XMMatrixDecompose(&childWorldS, &childWorldQ, &childWorldT, childWorldBind);
					XMMatrixDecompose(&childLocalS, &childLocalQ, &childLocalT, childLocalBind);

					skeletonMinXmV = XMVectorMin(skeletonMinXmV, childWorldT);
					skeletonMaxXmV = XMVectorMax(skeletonMaxXmV, childWorldT);

					const XMVECTOR lengthV = XMVector3Length(childLocalT);
					JVector3<float> lengthF = lengthV;
					joint[i].length = lengthF.x;
				}
				else
				{
					const XMMATRIX childWorldBind = XMMatrixInverse(nullptr, joint[i].inbindPose.LoadXM());
					XMVECTOR childWorldS;
					XMVECTOR childWorldQ;
					XMVECTOR childWorldT;
					XMMatrixDecompose(&childWorldS, &childWorldQ, &childWorldT, childWorldBind);

					skeletonMinXmV = XMVectorMin(skeletonMinXmV, childWorldT);
					skeletonMaxXmV = XMVectorMax(skeletonMaxXmV, childWorldT);

					JVector3<float> localTF = childWorldT;
					if (localTF.x != 0 && localTF.y != 0 && localTF.z != 0)
					{
						const XMVECTOR lengthV = XMVector3Length(childWorldT);
						JVector3<float> lengthF = lengthV;
						joint[i].length = lengthF.x;
					}
					else
						joint[i].length = 0;
				}
			}  
		}
		void JFbxFileLoader::StuffMaterial(FbxNode* node, JFbxMaterialMap& materialMap, const size_t meshGuid)
		{
			std::vector<JFbxMaterial> matVec;
			const uint matCount = node->GetMaterialCount();
			for (uint i = 0; i < matCount; ++i)
			{
				auto material = node->GetMaterial(i);
				auto mapData = dataSet->mateiralMap.find(material->GetName());
				if (mapData == dataSet->mateiralMap.end())
					continue;
				matVec.push_back(mapData->second);
			}
			materialMap.emplace(meshGuid, std::move(matVec));
		}
		FbxAMatrix JFbxFileLoader::GetGeometryTransformation(FbxNode* inNode)
		{
			if (!inNode)
			{
				J_LOG_PRINT_OUT("", "Invalid geometry"); 
				throw std::exception("Null for mesh geometry");
			}

			FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
			FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

			return FbxAMatrix(lT, lR, lS);
		}
		void JFbxFileLoader::CheckModelAxis(JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint)noexcept
		{
			int modelUpDir;
			FbxAxisSystem::EUpVector modelUpV;
			GetModelYUP(joint, modelUpDir, modelUpV);

			if (modelUpDir != Private::failUpDirSearch && (modelUpV != dataSet->sceneUpV || modelUpDir != dataSet->sceneUpDir))
				ConvertAxis(meshGroup, joint, modelUpDir, modelUpV);
		}
		void JFbxFileLoader::GetModelYUP(std::vector<Joint>& joint, int& outUpDir, FbxAxisSystem::EUpVector& outUpAxis)noexcept
		{
			XMMATRIX rootM = XMMatrixInverse(nullptr, joint[0].inbindPose.LoadXM());
			JMatrix4x4 rootFM;
			rootFM.StoreXM(rootM);

			XMMATRIX childM;
			JMatrix4x4 chilFM;
			uint i = 1;
			const uint jointCount = (uint)joint.size();
			for (; i < jointCount; ++i)
			{
				childM = XMMatrixInverse(nullptr, joint[i].inbindPose.LoadXM());
				chilFM.StoreXM(childM);

				if (chilFM._41 != 0 && chilFM._42 != 0 && chilFM._43 != 0)
					break;
			}

			if (i == jointCount)
			{
				outUpDir = Private::failUpDirSearch;
				return;
			}

			JVector3<float> rootT{ rootFM._41, rootFM._42, rootFM._43 };
			JVector3<float> childT{ chilFM._41, chilFM._42, chilFM._43 };
			//child joint(주로 root 다음은 heap) - root joint = dirVec 
			JVector3<float> gap = childT - rootT;

			if (abs(gap.x) >= abs(gap.y) && abs(gap.x) >= abs(gap.z))
			{
				if (gap.x > 0)
					outUpDir = 1;
				else
					outUpDir = -1;
				outUpAxis = FbxAxisSystem::EUpVector::eXAxis;
			}
			else if (abs(gap.y) >= abs(gap.x) && abs(gap.y) >= abs(gap.z))
			{
				if (gap.y > 0)
					outUpDir = 1;
				else
					outUpDir = -1;
				outUpAxis = FbxAxisSystem::EUpVector::eYAxis;
			}
			else if (abs(gap.z) >= abs(gap.x) && abs(gap.z) >= abs(gap.y))
			{
				if (gap.z > 0)
					outUpDir = 1;
				else
					outUpDir = -1;
				outUpAxis = FbxAxisSystem::EUpVector::eZAxis;
			}
		}
		void JFbxFileLoader::SetLeftHandMatrix(FbxAMatrix& m)noexcept
		{
			FbxVector4 t = m.GetT();
			FbxQuaternion q = m.GetQ();
			FbxVector4 s = m.GetS();

			SetLeftHandPosition(t);
			SetLeftHandQuaternion(q);
			SetLeftHandScale(s);
			q.Normalize();
			m.SetTQS(t, q, s);
		}
		void JFbxFileLoader::SetLeftHandPosition(FbxVector4& t)noexcept
		{
			if (dataSet->sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp)
				t.Set(-t.mData[0], t.mData[1], t.mData[2]);
			else if (dataSet->sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp)
				t.Set(-t.mData[0], t.mData[2], -t.mData[1]);
		}
		void JFbxFileLoader::SetLeftHandQuaternion(FbxQuaternion& q)noexcept
		{
			if (dataSet->sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp)
				q.Set(q.mData[0], -q.mData[1], -q.mData[2], q.mData[3]);
			else if (dataSet->sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp)
			{
				q.Set(q.mData[0], -q.mData[1], -q.mData[2], q.mData[3]);
				JVector4F zUpToYUpxMinus90 = Private::ZUpToYUpxMinus90();
				FbxQuaternion x90(zUpToYUpxMinus90.x, zUpToYUpxMinus90.y, zUpToYUpxMinus90.z, zUpToYUpxMinus90.w);
				q = x90 * q;
			}
		}
		void JFbxFileLoader::SetLeftHandScale(FbxVector4& s)noexcept
		{
			if (dataSet->sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp)
				s.Set(s.mData[0], s.mData[1], s.mData[2]);
			else if (dataSet->sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp)
				s.Set(s.mData[0], s.mData[2], s.mData[1]);
		}
		void JFbxFileLoader::SetRotationDegreeToRadian(FbxAMatrix& mat)noexcept
		{
			FbxVector4 r = mat.GetR();
			SetRotationDegreeToRadian(r);
			mat.SetR(r);
		}
		void JFbxFileLoader::SetRotationDegreeToRadian(FbxVector4& r)noexcept
		{
			r.Set(r.mData[0] * (JMathHelper::Pi / 180), r.mData[1] * (JMathHelper::Pi / 180),
				r.mData[2] * (JMathHelper::Pi / 180), r.mData[3]);
		}
		void JFbxFileLoader::SetSceneAxis(FbxScene* scene, FbxNode* root)noexcept
		{
			//Print3DProgramAxis()

			//FbxAxisSystem::OpenGL.ConvertChildren(root, FbxAxisSystem::OpenGL);
			//FbxAxisSystem::DirectX.DeepConvertScene(scene);
			dataSet->sceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
			dataSet->sceneUpDir = 0;
			dataSet->sceneFrontDir = 0;
			dataSet->sceneUpV = dataSet->sceneAxisSystem.GetUpVector(dataSet->sceneUpDir);
			dataSet->sceneFrontV = dataSet->sceneAxisSystem.GetFrontVector(dataSet->sceneFrontDir);
			dataSet->sceneCoordSys = dataSet->sceneAxisSystem.GetCoorSystem();

			if (dataSet->sceneCoordSys == FbxAxisSystem::ECoordSystem::eRightHanded)
			{
				if (dataSet->sceneUpV == FbxAxisSystem::EUpVector::eYAxis)
				{
					dataSet->sceneAxis = FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp;
					dataSet->sceneAxisSystem = FbxAxisSystem::MayaYUp;
				}
				else if (dataSet->sceneUpV == FbxAxisSystem::EUpVector::eZAxis)
				{
					dataSet->sceneAxis = FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp;
					dataSet->sceneAxisSystem = FbxAxisSystem::MayaZUp;
				}
			}
			else
			{
				dataSet->sceneAxis = FbxAxisSystem::EPreDefinedAxisSystem::eDirectX;
				dataSet->sceneAxisSystem = FbxAxisSystem::DirectX;
			}
		}
		void JFbxFileLoader::ConvertFbaToJM(FbxAMatrix& fbxM, JMatrix4x4& m)noexcept
		{
			for (int row = 0; row < 4; row++)
				for (int col = 0; col < 4; col++)
					m.m[row][col] = (float)fbxM.Get(row, col);

			JMathHelper::QuaternionNormalize(m);
		}
		void JFbxFileLoader::ConvertAxis(JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (dataSet->sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && dataSet->sceneUpDir == modelUpDir)
				{
					uint jointCount = (uint)joint.size();
					XMVECTOR sV;
					XMVECTOR qV;
					XMVECTOR tV;
					JVector3<float> sF;
					JVector4<float> qF;
					JVector3<float> tF;

					XMMATRIX bind;
					for (uint i = 0; i < jointCount; ++i)
					{
						bind = XMMatrixInverse(nullptr, joint[i].inbindPose.LoadXM());
						XMMatrixDecompose(&sV, &qV, &tV, bind);
						ConvertScale(sF, modelUpDir, modelUpV);
						ConvertQuaternion(qF, modelUpDir, modelUpV);
						ConvertPosition(tF, modelUpDir, modelUpV);
	 
						XMVECTOR zero = XMVectorSet(0, 0, 0, 1);
						joint[i].inbindPose.StoreXM(XMMatrixInverse(nullptr, XMMatrixAffineTransformation(sF.ToXmV(), zero, qF.ToXmV(), tF.ToXmV())));
					}
				}
				break;
			case FbxAxisSystem::EUpVector::eZAxis:
				break;
			default:
				break;
			}
			uint meshCount = meshGroup.GetMeshDataCount();
			for (uint i = 0; i < meshCount; ++i)
			{
				JSkinnedMeshData* mesdata = static_cast<JSkinnedMeshData*>(meshGroup.GetMeshData(i));
				const uint vertexCount = mesdata->GetVertexCount();
				for (uint j = 0; j < vertexCount; ++j)
				{
					JSkinnedMeshVertex vertex = mesdata->GetVertex(j);
					ConvertPosition(vertex.position, modelUpDir, modelUpV);
					ConvertPosition(vertex.normal, modelUpDir, modelUpV);
					ConvertPosition(vertex.tangentU, modelUpDir, modelUpV);
					mesdata->SetVertex(j, vertex);
				}
				mesdata->CreateBoundingObject();
			}
		}
		void JFbxFileLoader::ConvertPosition(JVector3<float>& position, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (dataSet->sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && modelUpDir == dataSet->sceneUpDir)
				{
					JVector3<float> oldPosition = position;
					position.y = oldPosition.z;
					position.z = -oldPosition.y;
				}
				break;
			case FbxAxisSystem::EUpVector::eZAxis:
				break;
			default:
				break;
			}
		}
		void JFbxFileLoader::ConvertQuaternion(JVector4<float>& quaternion, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (dataSet->sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && modelUpDir == dataSet->sceneUpDir)
				{
					XMVECTOR modQ = XMQuaternionMultiply(Private::ZUpToYUpxMinus90().ToXmV(), quaternion.ToXmV());
					quaternion = modQ;
				}
				break;
			case FbxAxisSystem::EUpVector::eZAxis:
				break;
			default:
				break;
			}
		}
		void JFbxFileLoader::ConvertScale(JVector3<float>& scale, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (dataSet->sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && modelUpDir == dataSet->sceneUpDir)
				{
					JVector3<float> oldScale = scale;
					scale.y = oldScale.z;
					scale.z = oldScale.y;
				}
				break;
			case FbxAxisSystem::EUpVector::eZAxis:
				break;
			default:
				break;
			}
		}
		size_t JFbxFileLoader::MakeVertexMapKey(const DirectX::XMFLOAT3& positionXm,
			const DirectX::XMFLOAT2& textureXm,
			const DirectX::XMFLOAT3& normalXm,
			const DirectX::XMFLOAT3& biNormalXm,
			const DirectX::XMFLOAT4& tangentXm,
			const bool isSkin)noexcept
		{
			std::string hashKey;
			hashKey += "x:" + std::to_string(positionXm.x) +
				"y:" + std::to_string(positionXm.y) +
				"z:" + std::to_string(positionXm.z);
			hashKey += "x:" + std::to_string(textureXm.x) +
				"y:" + std::to_string(textureXm.y);
			hashKey += "x:" + std::to_string(normalXm.x) +
				"y:" + std::to_string(normalXm.y) +
				"z:" + std::to_string(normalXm.z);
			hashKey += "x:" + std::to_string(biNormalXm.x) +
				"y:" + std::to_string(biNormalXm.y) +
				"z:" + std::to_string(biNormalXm.z);
			hashKey += "x:" + std::to_string(tangentXm.x) +
				"y:" + std::to_string(tangentXm.y) +
				"z:" + std::to_string(tangentXm.z) +
				"w:" + std::to_string(tangentXm.w);
			hashKey += "isSkin:" + std::to_string(isSkin);
			return JCUtil::CalculateGuid(hashKey);
		}
		void JFbxFileLoader::ResizeMatrix(JMatrix4x4& m)noexcept
		{
			m._41 *= dataSet->resizeRate;
			m._42 *= dataSet->resizeRate;
			m._43 *= dataSet->resizeRate;
		}
		void JFbxFileLoader::ResizeVertexPosition(JVector3<float>& vertexPosition)noexcept
		{
			vertexPosition.x *= dataSet->resizeRate;
			vertexPosition.y *= dataSet->resizeRate;
			vertexPosition.z *= dataSet->resizeRate;
		} 
	}
}
/*

//JDebugTextOut::PrintWstr(JCUtil::StrToWstr("P: "+ skeleton.joint[parentIndex].name));
//JDebugTextOut::PrintWstr(JCUtil::StrToWstr("C: "+ skeleton.joint[i].name));
//JDebugTextOut::PrintMatrixAndVector(childWorldBind, L"childWorldBind");
//JDebugTextOut::PrintMatrixAndVector(childLocalBind, L"childLocalBind");
//JDebugTextOut::PrintVector(lengthV, L"lengthV");
*/

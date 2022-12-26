#include"JFbxFileLoader.h"    
#include"../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../../../Utility/JMathHelper.h" 
#include"../../../Utility/JCommonUtility.h"
#include"../../Geometry/JDirectXCollisionEx.h"
#include"../../Guid/GuidCreator.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{
		JFbxFileLoaderImpl::~JFbxFileLoaderImpl()
		{
			if (fbxManager != nullptr)
			{
				fbxManager->Destroy();
			}
		}
		J_FBXRESULT JFbxFileLoaderImpl::LoadFbxMeshFile(const std::string& path, JStaticMeshGroup& meshGroup)
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
				return J_FBXRESULT::FAIL;
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");

			//fbx file 내용을 scene으로 가져온다
			if (importer->Import(scene))
			{
				//++num;

				// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
				FbxNode* rootNode = scene->GetRootNode();
				SetSceneAxis(scene, rootNode);

				FbxGeometryConverter geometryConverter(fbxManager);
				geometryConverter.Triangulate(scene, true);
				J_FBXRESULT result;

				vertexIndexMap.clear();
				vertexCount = 0; 
				LoadNode(rootNode, meshGroup);

				if (meshGroup.GetMeshDataCount() > 0)
					result = J_FBXRESULT::HAS_MESH;

				controlPoint.clear();
				vertexIndexMap.clear();
				scene->Destroy();
				importer->Destroy();
				return result;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return J_FBXRESULT::FAIL;
			}
		}
		J_FBXRESULT JFbxFileLoaderImpl::LoadFbxMeshFile(const std::string& path, JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint)
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
				return J_FBXRESULT::FAIL;
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");

			//fbx file 내용을 scene으로 가져온다
			if (importer->Import(scene))
			{
				//++num;

				// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
				FbxNode* rootNode = scene->GetRootNode();
				SetSceneAxis(scene, rootNode);

				FbxGeometryConverter geometryConverter(fbxManager);
				geometryConverter.Triangulate(scene, true);
				J_FBXRESULT result;

				vertexIndexMap.clear();
				vertexCount = 0;
				//hasRootJoint = true;

				JFbxSkeleton fbxSkeleton;
				LoadJoint(rootNode, -1, -1, -1, fbxSkeleton);

				//ProcessSkeletonHierarchy(rootNode, fbxSkeleton);
				//fbxSkeleton.joint[0].parentIndex == 0 add emptyNode in front
				bool hasSkeleton = fbxSkeleton.jointCount > 0;			 
				LoadNode(rootNode, meshGroup, fbxSkeleton);

				if (hasSkeleton)
				{
					StuffSkletonData(fbxSkeleton, joint);
					CheckModelAxis(meshGroup, joint);
					result = (J_FBXRESULT)((int)result | (int)J_FBXRESULT::HAS_SKELETON);
				}

				fbxSkeleton.joint.clear();
				controlPoint.clear();
				vertexIndexMap.clear();
				scene->Destroy();
				importer->Destroy();
				return result;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return J_FBXRESULT::FAIL;
			}
		}
		J_FBXRESULT JFbxFileLoaderImpl::LoadFbxAnimationFile(const std::string& path, JFbxAnimationData& jfbxAniData)
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
				return J_FBXRESULT::FAIL;
			}
			FbxScene* scene = FbxScene::Create(fbxManager, "JScene");

			//fbx file 내용을 scene으로 가져온다
			if (importer->Import(scene))
			{
				//++num;

				// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
				FbxNode* rootNode = scene->GetRootNode();
				SetSceneAxis(scene, rootNode);

				FbxGeometryConverter geometryConverter(fbxManager);
				geometryConverter.Triangulate(scene, true);
				J_FBXRESULT result;

				vertexIndexMap.clear();
				vertexCount = 0;
				//hasRootJoint = true;

				JFbxSkeleton fbxSkeleton;
				LoadJoint(rootNode, -1, -1, -1, fbxSkeleton);

				//ProcessSkeletonHierarchy(rootNode, fbxSkeleton);
				//fbxSkeleton.joint[0].parentIndex == 0 add emptyNode in front

				bool hasSkeleton = (fbxSkeleton.jointCount > 0);
				result = LoadAnimationClip(scene, rootNode, fbxSkeleton, hasSkeleton, jfbxAniData);

				if (hasSkeleton)
				{ 
					result = (J_FBXRESULT)((int)result | (int)J_FBXRESULT::HAS_SKELETON);
					std::string totalName;
					for (uint i = 0; i < fbxSkeleton.jointCount; ++i)
						totalName += fbxSkeleton.joint[i].name;
					jfbxAniData.skeletonHash = JCUtil::CalculateGuid(totalName);
				}
				fbxSkeleton.joint.clear();
				controlPoint.clear();
				vertexIndexMap.clear();
				scene->Destroy();
				importer->Destroy();
				return result;
			}
			else
			{
				scene->Destroy();
				importer->Destroy();
				return J_FBXRESULT::FAIL;
			}
		}
		JFbxFileLoaderImpl::FbxFileTypeInfo JFbxFileLoaderImpl::GetFileTypeInfo(const std::string& path)
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

				FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();
				if (animStack)
				{
					res.animName = animStack->GetName();
					res.typeInfo = J_FBXRESULT::HAS_ANIMATION;
				}

				uint meshCount;
				uint jointCount;
				GetMeshCount(scene->GetRootNode(), meshCount, res);
				GetJointCount(scene->GetRootNode(), jointCount, res);

				if (meshCount > 0)
					res.typeInfo = (J_FBXRESULT)((int)res.typeInfo | (int)J_FBXRESULT::HAS_MESH);
				if (jointCount > 0)
					res.typeInfo = (J_FBXRESULT)((int)res.typeInfo | (int)J_FBXRESULT::HAS_SKELETON);

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
		void JFbxFileLoaderImpl::GetMeshCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo)
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
		void JFbxFileLoaderImpl::GetJointCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo)
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
		void JFbxFileLoaderImpl::LoadNode(FbxNode* node, JStaticMeshGroup& meshGroup)
		{
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
			if (nodeAttribute)
			{
				if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					LoadControlPoint(node->GetMesh()); 
					SortBlendingWeight();
					JStaticMeshData staticMesh = LoadStaticMesh(node);
					if (staticMesh.IsValid())
						meshGroup.AddMeshData(std::move(staticMesh));
				}
			}
			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				LoadNode(node->GetChild(i), meshGroup);
		}
		void JFbxFileLoaderImpl::LoadNode(FbxNode* node, JSkinnedMeshGroup& meshGroup, JFbxSkeleton& skeleton)
		{
			FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
			if (nodeAttribute)
			{
				if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					LoadControlPoint(node->GetMesh());
					LoadSkinnedMeshInfo(node, skeleton);
					SortBlendingWeight();
					  
					JSkinnedMeshData skinnedMesh = LoadSkinnedMesh(node, skeleton);
					if (skinnedMesh.IsValid())
						meshGroup.AddMeshData(std::move(skinnedMesh));
				}
			}
			const uint childCount = node->GetChildCount();
			for (uint i = 0; i < childCount; ++i)
				LoadNode(node->GetChild(i), meshGroup, skeleton);
		}
		void JFbxFileLoaderImpl::LoadJoint(FbxNode* node, int depth, int index, int parentIndex, JFbxSkeleton& skeleton)
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
					rootjoint.max = JVector3<float>(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
					rootjoint.min = JVector3<float>(JMathHelper::Infinity, JMathHelper::Infinity, JMathHelper::Infinity);
					skeleton.joint.push_back(rootjoint);
					++skeleton.jointCount;
					++index;
					//hasRootJoint = false;
				}
				JFbxJoint joint;
				joint.parentIndex = parentIndex;
				joint.name = node->GetName();
				joint.node = node;
				joint.max = JVector3<float>(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
				joint.min = JVector3<float>(JMathHelper::Infinity, JMathHelper::Infinity, JMathHelper::Infinity);
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
		JStaticMeshData JFbxFileLoaderImpl::LoadStaticMesh(FbxNode* node)
		{
			FbxMesh* mesh = node->GetMesh();
			if (!mesh->GetNode())
				return JStaticMeshData{};
 
			std::vector<JStaticMeshVertex> vertices;
			std::vector<uint32> indices;

			uint triangleCount = mesh->GetPolygonCount();
			uint idx[3];
			vertexCount = 0;

			for (uint i = 0; i < triangleCount; ++i)
			{
				bool hasNormal;
				bool hasUv;
				bool hasBinormal;
				bool hasTangent;
				for (int j = 0; j < 3; ++j)
				{
					uint index = indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);
					idx[j] = index;
					positionXm[j] = controlPoint[controlPointIndex].position.ConvertXMF();

					hasUv = LoadTextureUV(mesh, controlPointIndex, mesh->GetTextureUVIndex(i, index), textureXm[j]);
					hasNormal = LoadNormal(mesh, controlPointIndex, vertexCount, normalXm[j]);
					hasBinormal = LoadBinormal(mesh, controlPointIndex, vertexCount, biNormalXm[j]);
					hasTangent = LoadTangent(mesh, controlPointIndex, vertexCount, tangentXm[j]);
					textureXm[j].y = 1 - textureXm[j].y; 
					++vertexCount;
				}

				if (!hasTangent)
				{
					ComputeTangentFrame(idx, 1, positionXm, normalXm, textureXm, 3, tangentXm);
				}
				 
				for (int j = 0; j < 3; ++j)
				{
					uint index = indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);

					JStaticMeshVertex newVertex(positionXm[j], normalXm[j], textureXm[j], tangentXm[j]);
					size_t guid = MakeVertexMapKey(positionXm[j], textureXm[j], normalXm[j], biNormalXm[j], tangentXm[j], controlPoint[controlPointIndex].isSkin);
				
					auto data = vertexIndexMap.find(guid);
					if (data != vertexIndexMap.end())
						indices.push_back(data->second);
					else
					{
						uint vertexIndex = (uint)vertices.size();
						vertexIndexMap.emplace(guid, vertexIndex);
						indices.push_back(vertexIndex);
						vertices.push_back(newVertex);
					}
				}
			}
			return JStaticMeshData{ JCUtil::StrToWstr(node->GetName()), MakeGuid(), std::move(indices), true, true, std::move(vertices)};
		}
		JSkinnedMeshData JFbxFileLoaderImpl::LoadSkinnedMesh(FbxNode* node, JFbxSkeleton& skeleton)
		{
			FbxMesh* mesh = node->GetMesh();
			if (!mesh->GetNode())
				return JSkinnedMeshData{};

			std::vector<JSkinnedMeshVertex> vertices;
			std::vector<uint32> indices;

			uint triangleCount = mesh->GetPolygonCount();
			uint idx[3];
			vertexCount = 0;
			vertexIndexMap.clear();

			for (uint i = 0; i < triangleCount; ++i)
			{
				bool hasNormal = false;
				bool hasUv = false;
				bool hasBinormal = false;
				bool hasTangent = false;
				for (int j = 0; j < 3; ++j)
				{
					uint index = indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);
					idx[j] = index;
					positionXm[j] = controlPoint[controlPointIndex].position.ConvertXMF();

					hasUv = LoadTextureUV(mesh, controlPointIndex, mesh->GetTextureUVIndex(i, index), textureXm[j]);
					hasNormal = LoadNormal(mesh, controlPointIndex, vertexCount, normalXm[j]);
					hasBinormal = LoadBinormal(mesh, controlPointIndex, vertexCount, biNormalXm[j]);
					hasTangent = LoadTangent(mesh, controlPointIndex, vertexCount, tangentXm[j]);

					textureXm[j].y = 1 - textureXm[j].y; 
					++vertexCount;
				}

				if (!hasTangent)
					ComputeTangentFrame(idx, 1, positionXm, normalXm, textureXm, 3, tangentXm);

				for (int j = 0; j < 3; ++j)
				{
					uint index = indexingOrder[j];
					int controlPointIndex = mesh->GetPolygonVertex(i, index);

					size_t guid = MakeVertexMapKey(positionXm[j], textureXm[j], normalXm[j], biNormalXm[j], tangentXm[j], controlPoint[controlPointIndex].isSkin);
					auto data = vertexIndexMap.find(guid);
					
					if (data != vertexIndexMap.end())
						indices.push_back(data->second);
					else
					{  
						JSkinnedMeshVertex newVertex(positionXm[j], normalXm[j], textureXm[j], tangentXm[j], controlPoint[controlPointIndex].blendingInfo);
						uint32 vertexIndex = (uint32)vertices.size();
						vertexIndexMap.emplace(guid, vertexIndex);

						indices.push_back(vertexIndex);
						vertices.push_back(newVertex);

						JVector3 pos = JVector3<float>(positionXm[j].x, positionXm[j].y, positionXm[j].z);
						for (int k = 0; k < controlPoint[controlPointIndex].blendingInfo.size(); ++k)
						{
							int jointIndex = controlPoint[controlPointIndex].blendingInfo[k].blendingIndex;
							if (controlPoint[controlPointIndex].blendingInfo[k].blendingWeight > 0.5f)
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

			return JSkinnedMeshData{JCUtil::StrToWstr(node->GetName()), MakeGuid(), std::move(indices), true, true, std::move(vertices) };
		}
		void JFbxFileLoaderImpl::LoadControlPoint(FbxMesh* mesh)
		{
			uint count = mesh->GetControlPointsCount();

			controlPoint.clear();
			controlPoint.resize(count);
			FbxVector4 trans;
			for (uint i = 0; i < count; ++i)
			{
				trans = mesh->GetControlPointAt(i);
				SetLeftHandPosition(trans);
				controlPoint[i].position.x = static_cast<float>(trans[0]);
				controlPoint[i].position.y = static_cast<float>(trans[1]);
				controlPoint[i].position.z = static_cast<float>(trans[2]);
				ResizeVertexPosition(controlPoint[i].position);
			}
		}
		bool JFbxFileLoaderImpl::LoadTextureUV(const FbxMesh* mesh, int controlPointIndex, int inTextureUVIndex, XMFLOAT2& outUV)
		{
			outUV = JMathHelper::Zero2f();
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
		bool JFbxFileLoaderImpl::LoadNormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, XMFLOAT3& outNormal)
		{
			outNormal = JMathHelper::Zero3f();
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
		bool JFbxFileLoaderImpl::LoadBinormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, XMFLOAT3& outBinormal)
		{
			outBinormal = JMathHelper::Zero3f();
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
		bool JFbxFileLoaderImpl::LoadTangent(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, XMFLOAT4& outTangent)
		{
			outTangent = JMathHelper::Zero4f();
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
		void JFbxFileLoaderImpl::LoadSkinnedMeshInfo(FbxNode* node, JFbxSkeleton& skeleton)
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
						controlPoint[pCtrlPtIdx[k]].blendingInfo.push_back(blend);
						controlPoint[pCtrlPtIdx[k]].isSkin = true;
					}
				}
			}
			JBlendingIndexWeightPair blendingIndexWeightPair;
			blendingIndexWeightPair.blendingIndex = 0;
			blendingIndexWeightPair.blendingWeight = 0;
			uint controlPointSize = (uint)controlPoint.size();
			for (uint i = 0; i < controlPointSize; ++i)
			{
				for (uint j = (uint)controlPoint[i].blendingInfo.size(); j < 4; ++j)
				{
					controlPoint[i].blendingInfo.push_back(blendingIndexWeightPair);
				}
			}
		}
		J_FBXRESULT JFbxFileLoaderImpl::LoadAnimationClip(FbxScene* scene, FbxNode* node, JFbxSkeleton& skeleton, bool hasSkeleton, JFbxAnimationData& fbxAniData)
		{
			FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>();
			if (!animStack)
				return J_FBXRESULT::FAIL;

			int animStackcnt = scene->GetSrcObjectCount< FbxAnimStack>();
			FbxString animStackName = animStack->GetName();
			FbxTakeInfo* takeInfo = scene->GetTakeInfo(animStackName);
			FbxTime animationStart = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime animationEnd = takeInfo->mLocalTimeSpan.GetStop();
			size_t animationLength = animationEnd.GetFrameCount(FbxTime::eFrames24) - animationStart.GetFrameCount(FbxTime::eFrames24) + 1;
			FbxAMatrix geometryTransform = GetGeometryTransformation(node);
			FbxAMatrix InvBoneInitPoseGlobalTransform;
			FbxAMatrix matBeforeMatrix;
			FbxAMatrix matNowMatrix;
			FbxAMatrix currentTransformOffset;
			XMFLOAT4X4 xmMatrix;

			fbxAniData.clipLength = (uint)animationLength;
			fbxAniData.framePerSecond = 24;
			fbxAniData.animationSample.resize(skeleton.jointCount);

			for (uint i = 0; i < skeleton.jointCount; ++i)
			{
				std::string pName = skeleton.joint[i].name;
				matBeforeMatrix.SetIdentity();

				for (FbxLongLong j = animationStart.GetFrameCount(FbxTime::eFrames24);
					j <= animationEnd.GetFrameCount(FbxTime::eFrames24); ++j)
				{
					FbxTime currTime;
					currTime.SetFrame(j, FbxTime::eFrames24);

					//InvBoneInitPoseGlobalTransform = skeleton.joint[0].node->EvaluateGlobalTransform(currTime) *
					//	geometryTransform; 
					matNowMatrix = scene->GetAnimationEvaluator()->GetNodeGlobalTransform(skeleton.joint[i].node, currTime) * geometryTransform;
					SetLeftHandMatrix(matNowMatrix);
					if (matNowMatrix == matBeforeMatrix)
						continue;

					matBeforeMatrix = matNowMatrix;
					ConvertFbaToXM(matNowMatrix, xmMatrix);
					ResizeMatrix(xmMatrix);
					fbxAniData.animationSample[i].jointPose.emplace_back(xmMatrix, (float)currTime.GetSecondDouble());
				}
			}
			return J_FBXRESULT::HAS_ANIMATION;
		}
		void JFbxFileLoaderImpl::SortBlendingWeight()
		{
			uint count = (uint)controlPoint.size();
			for (uint i = 0; i < count; ++i)
			{
				sort(controlPoint[i].blendingInfo.begin(), controlPoint[i].blendingInfo.end(), CompareBlendingWeight);
			}
		}
		void JFbxFileLoaderImpl::StuffSkletonData(JFbxSkeleton& fbxSkeleton, std::vector<Joint>& joint)
		{  
			joint.reserve(fbxSkeleton.jointCount);
			joint.resize(fbxSkeleton.jointCount);

			XMFLOAT3 skeletonMinInit(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity);
			XMFLOAT3 skeletonMaxInit(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
			XMVECTOR skeletonMinXmV = XMLoadFloat3(&skeletonMinInit);
			XMVECTOR skeletonMaxXmV = XMLoadFloat3(&skeletonMaxInit);

			for (uint i = 0; i < fbxSkeleton.jointCount; ++i)
			{
				joint[i].name = JCUtil::StrToWstr(fbxSkeleton.joint[i].name);
				joint[i].parentIndex = fbxSkeleton.joint[i].parentIndex;
				ConvertFbaToXM(fbxSkeleton.joint[i].globalBindPositionInverse, joint[i].inbindPose);
				ResizeMatrix(joint[i].inbindPose);

				if (i > 0)
				{
					const uint8 parentIndex = joint[i].parentIndex;
					const XMMATRIX parentWorldBind = XMMatrixInverse(nullptr, XMLoadFloat4x4(&joint[parentIndex].inbindPose));
					const XMMATRIX childWorldBind = XMMatrixInverse(nullptr, XMLoadFloat4x4(&joint[i].inbindPose));
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
					XMFLOAT3 lengthF;
					XMStoreFloat3(&lengthF, lengthV);
					joint[i].length = lengthF.x;
				}
				else
				{
					const XMMATRIX childWorldBind = XMMatrixInverse(nullptr, XMLoadFloat4x4(&joint[i].inbindPose));
					XMVECTOR childWorldS;
					XMVECTOR childWorldQ;
					XMVECTOR childWorldT;
					XMMatrixDecompose(&childWorldS, &childWorldQ, &childWorldT, childWorldBind);

					skeletonMinXmV = XMVectorMin(skeletonMinXmV, childWorldT);
					skeletonMaxXmV = XMVectorMax(skeletonMaxXmV, childWorldT);

					XMFLOAT3 localTF;
					XMStoreFloat3(&localTF, childWorldT);
					if (localTF.x != 0 && localTF.y != 0 && localTF.z != 0)
					{
						const XMVECTOR lengthV = XMVector3Length(childWorldT);
						XMFLOAT3 lengthF;
						XMStoreFloat3(&lengthF, lengthV);
						joint[i].length = lengthF.x;
					}
					else
						joint[i].length = 0;
				}
			}  
		}
		FbxAMatrix JFbxFileLoaderImpl::GetGeometryTransformation(FbxNode* inNode)
		{
			if (!inNode)
			{
				MessageBox(0, L"Null for mesh geometry", 0, 0);
				throw std::exception("Null for mesh geometry");
			}

			FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
			FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

			return FbxAMatrix(lT, lR, lS);
		}
		void JFbxFileLoaderImpl::CheckModelAxis(JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint)noexcept
		{
			int modelUpDir;
			FbxAxisSystem::EUpVector modelUpV;
			GetModelYUP(joint, modelUpDir, modelUpV);

			if (modelUpDir != failUpDirSearch && (modelUpV != sceneUpV || modelUpDir != sceneUpDir))
				ConvertAxis(meshGroup, joint, modelUpDir, modelUpV);
		}
		void JFbxFileLoaderImpl::GetModelYUP(std::vector<Joint>& joint, int& outUpDir, FbxAxisSystem::EUpVector& outUpAxis)noexcept
		{
			XMMATRIX rootM = XMMatrixInverse(nullptr, XMLoadFloat4x4(&joint[0].inbindPose));
			XMFLOAT4X4 rootFM;
			XMStoreFloat4x4(&rootFM, rootM);

			XMMATRIX childM;
			XMFLOAT4X4 chilFM;
			uint i = 1;
			const uint jointCount = (uint)joint.size();
			for (; i < jointCount; ++i)
			{
				childM = XMMatrixInverse(nullptr, XMLoadFloat4x4(&joint[i].inbindPose));
				XMStoreFloat4x4(&chilFM, childM);

				if (chilFM._41 != 0 && chilFM._42 != 0 && chilFM._43 != 0)
					break;
			}

			if (i == jointCount)
			{
				outUpDir = failUpDirSearch;
				return;
			}

			XMFLOAT3 rootT{ rootFM._41, rootFM._42, rootFM._43 };
			XMFLOAT3 childT{ chilFM._41, chilFM._42, chilFM._43 };
			//child joint(주로 root 다음은 heap) - root joint = dirVec 
			XMFLOAT3 gap = JMathHelper::Vector3Minus(childT, rootT);

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
		void JFbxFileLoaderImpl::SetLeftHandMatrix(FbxAMatrix& m)noexcept
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
		void JFbxFileLoaderImpl::SetLeftHandPosition(FbxVector4& t)noexcept
		{
			if (sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp)
				t.Set(-t.mData[0], t.mData[1], t.mData[2]);
			else if (sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp)
				t.Set(-t.mData[0], t.mData[2], -t.mData[1]);
		}
		void JFbxFileLoaderImpl::SetLeftHandQuaternion(FbxQuaternion& q)noexcept
		{
			if (sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp)
				q.Set(q.mData[0], -q.mData[1], -q.mData[2], q.mData[3]);
			else if (sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp)
			{
				q.Set(q.mData[0], -q.mData[1], -q.mData[2], q.mData[3]);
				FbxQuaternion x90{ ZUpToYUpxMinus90.x, ZUpToYUpxMinus90.y, ZUpToYUpxMinus90.z, ZUpToYUpxMinus90.w };
				q = x90 * q;
			}
		}
		void JFbxFileLoaderImpl::SetLeftHandScale(FbxVector4& s)noexcept
		{
			if (sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp)
				s.Set(s.mData[0], s.mData[1], s.mData[2]);
			else if (sceneAxis == FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp)
				s.Set(s.mData[0], s.mData[2], s.mData[1]);
		}
		void JFbxFileLoaderImpl::SetRotationDegreeToRadian(FbxAMatrix& mat)noexcept
		{
			FbxVector4 r = mat.GetR();
			SetRotationDegreeToRadian(r);
			mat.SetR(r);
		}
		void JFbxFileLoaderImpl::SetRotationDegreeToRadian(FbxVector4& r)noexcept
		{
			r.Set(r.mData[0] * (JMathHelper::Pi / 180), r.mData[1] * (JMathHelper::Pi / 180),
				r.mData[2] * (JMathHelper::Pi / 180), r.mData[3]);
		}
		void JFbxFileLoaderImpl::SetSceneAxis(FbxScene* scene, FbxNode* root)noexcept
		{
			//Print3DProgramAxis()

			//FbxAxisSystem::OpenGL.ConvertChildren(root, FbxAxisSystem::OpenGL);
			//FbxAxisSystem::DirectX.DeepConvertScene(scene);
			sceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
			sceneUpDir = 0;
			sceneFrontDir = 0;
			sceneUpV = sceneAxisSystem.GetUpVector(sceneUpDir);
			sceneFrontV = sceneAxisSystem.GetFrontVector(sceneFrontDir);
			sceneCoordSys = sceneAxisSystem.GetCoorSystem();

			if (sceneCoordSys == FbxAxisSystem::ECoordSystem::eRightHanded)
			{
				if (sceneUpV == FbxAxisSystem::EUpVector::eYAxis)
				{
					sceneAxis = FbxAxisSystem::EPreDefinedAxisSystem::eMayaYUp;
					sceneAxisSystem = FbxAxisSystem::MayaYUp;
				}
				else if (sceneUpV == FbxAxisSystem::EUpVector::eZAxis)
				{
					sceneAxis = FbxAxisSystem::EPreDefinedAxisSystem::eMayaZUp;
					sceneAxisSystem = FbxAxisSystem::MayaZUp;
				}
			}
			else
			{
				sceneAxis = FbxAxisSystem::EPreDefinedAxisSystem::eDirectX;
				sceneAxisSystem = FbxAxisSystem::DirectX;
			}
		}
		void JFbxFileLoaderImpl::ConvertFbaToXM(FbxAMatrix& fbxM, XMFLOAT4X4& xmM)noexcept
		{
			for (int row = 0; row < 4; row++)
				for (int col = 0; col < 4; col++)
					xmM.m[row][col] = (float)fbxM.Get(row, col);

			JMathHelper::QuaternionNormalize(xmM);
		}
		void JFbxFileLoaderImpl::ConvertAxis(JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && sceneUpDir == modelUpDir)
				{
					uint jointCount = (uint)joint.size();
					XMVECTOR sV;
					XMVECTOR qV;
					XMVECTOR tV;
					XMFLOAT3 sF;
					XMFLOAT4 qF;
					XMFLOAT3 tF;

					XMMATRIX bind;
					for (uint i = 0; i < jointCount; ++i)
					{
						bind = XMMatrixInverse(nullptr, XMLoadFloat4x4(&joint[i].inbindPose));
						XMMatrixDecompose(&sV, &qV, &tV, bind);
						XMStoreFloat3(&sF, sV);
						XMStoreFloat4(&qF, qV);
						XMStoreFloat3(&tF, tV);

						ConvertScale(sF, modelUpDir, modelUpV);
						ConvertQuaternion(qF, modelUpDir, modelUpV);
						ConvertPosition(tF, modelUpDir, modelUpV);

						sV = XMLoadFloat3(&sF);
						qV = XMLoadFloat4(&qF);
						tV = XMLoadFloat3(&tF);

						XMVECTOR zero = XMVectorSet(0, 0, 0, 1);
						XMStoreFloat4x4(&joint[i].inbindPose, XMMatrixInverse(nullptr, XMMatrixAffineTransformation(sV, zero, qV, tV)));
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
		void JFbxFileLoaderImpl::ConvertPosition(XMFLOAT3& position, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && modelUpDir == sceneUpDir)
				{
					XMFLOAT3 oldPosition = position;
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
		void JFbxFileLoaderImpl::ConvertQuaternion(XMFLOAT4& quaternion, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && modelUpDir == sceneUpDir)
				{
					XMVECTOR modQ = XMQuaternionMultiply(XMLoadFloat4(&ZUpToYUpxMinus90), XMLoadFloat4(&quaternion));
					XMStoreFloat4(&quaternion, modQ);
				}
				break;
			case FbxAxisSystem::EUpVector::eZAxis:
				break;
			default:
				break;
			}
		}
		void JFbxFileLoaderImpl::ConvertScale(XMFLOAT3& scale, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept
		{
			switch (sceneUpV)
			{
			case FbxAxisSystem::EUpVector::eXAxis:
				break;
			case FbxAxisSystem::EUpVector::eYAxis:
				if (modelUpV == FbxAxisSystem::EUpVector::eZAxis && modelUpDir == sceneUpDir)
				{
					XMFLOAT3 oldScale = scale;
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
		size_t JFbxFileLoaderImpl::MakeVertexMapKey(const XMFLOAT3& positionXm,
			const XMFLOAT2& textureXm,
			const XMFLOAT3& normalXm,
			const XMFLOAT3& biNormalXm,
			const XMFLOAT4& tangentXm,
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
		void JFbxFileLoaderImpl::ResizeMatrix(XMFLOAT4X4& xmF)noexcept
		{
			//xmF._41 *= resizeRate;
			//xmF._42 *= resizeRate;
			//xmF._43 *= resizeRate;
		}
		void JFbxFileLoaderImpl::ResizeVertexPosition(XMFLOAT3& vertexPosition)noexcept
		{
			//vertexPosition.x *= resizeRate;
			//vertexPosition.y *= resizeRate;
			//vertexPosition.z *= resizeRate;
		}
		void JFbxFileLoaderImpl::ResizeVertexPosition(JVector3<float>& vertexPosition)noexcept
		{
			//vertexPosition.x *= resizeRate;
			//vertexPosition.y *= resizeRate;
			//vertexPosition.z *= resizeRate;
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

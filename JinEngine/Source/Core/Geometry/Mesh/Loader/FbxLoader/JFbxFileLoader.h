#pragma once
#include<winerror.h>
#include<fstream>
#include<algorithm>
#include"JFbxUtility.h"
#include"JFbxResult.h" 
#include"../../../../Singleton/JSingletonHolder.h"

namespace JinEngine
{  
	class JMaterial;
	class JTexture;
	namespace Core
	{
		struct Joint; 
		using JFbxMaterialMap = std::unordered_map<size_t, std::vector<JFbxMaterial>>;
		class JFbxFileLoader
		{
		public:
			struct FbxFileTypeInfo
			{
			public:
				J_FBX_RESULT typeInfo = J_FBX_RESULT_FAIL;
				std::string meshRootName;
				std::string skeletonRootName;
				std::string animName; 
			public:
				uint meshCount = 0;
				uint jointCount = 0;
				uint materialCount = 0;
			}; 
			struct FbxLoadDataSet
			{
			public:
				std::unordered_map<size_t, uint32> vertexIndexMap; 
				std::unordered_map<std::string, JFbxMaterial> mateiralMap;		//key is meterialName
				std::vector<JFbxControlPoint> controlPoint;
				std::vector<JBlendingIndexWeightPair> nowBlendingPair[3];
				uint indexingOrder[3] = { 0,1,2 };
				bool isSkinMesh[3];
				uint vertexCount = 0;
				FbxAxisSystem::EUpVector sceneUpV;
				FbxAxisSystem::EFrontVector sceneFrontV;
				FbxAxisSystem::ECoordSystem sceneCoordSys;
				FbxAxisSystem::EPreDefinedAxisSystem sceneAxis;
				FbxAxisSystem sceneAxisSystem;
				int sceneUpDir = 0;
				int sceneFrontDir = 0;
				float resizeRate = 1;
			public:
				//Debug 
				int num = 0;
			};
		private:
			FbxManager* fbxManager;
			std::unique_ptr<FbxLoadDataSet> dataSet;
		private:
			//Debug 
			int num = 0;
		public:
			~JFbxFileLoader();
			J_FBX_RESULT LoadFbxMeshFile(const std::string& path, JStaticMeshGroup& meshGroup, JFbxMaterialMap& materialMap);
			J_FBX_RESULT LoadFbxMeshFile(const std::string& path, JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint, JFbxMaterialMap& materialMap);
			J_FBX_RESULT LoadFbxAnimationFile(const std::string& path, JFbxAnimationData& jfbxAniData);
			FbxFileTypeInfo GetFileTypeInfo(const std::string& path);
		private:
			void GetMeshCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo);
			void GetJointCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo);
			void LoadNode(FbxNode* node, JStaticMeshGroup& meshGroup, JFbxMaterialMap& materialMap);
			void LoadNode(FbxNode* node, JSkinnedMeshGroup& meshGroup, JFbxSkeleton& skeleton, JFbxMaterialMap& materialMap);
			void LoadJoint(FbxNode* node, int depth, int index, int parentIndex, JFbxSkeleton& skeleton);
			std::unique_ptr<JStaticMeshData> LoadStaticMesh(FbxNode* node);
			std::unique_ptr<JSkinnedMeshData> LoadSkinnedMesh(FbxNode* node, JFbxSkeleton& skeleton);
			void LoadControlPoint(FbxMesh* mesh);
			bool LoadTextureUV(const FbxMesh* mesh, int controlPointIndex, int inTextureUVIndex, DirectX::XMFLOAT2& outUV);
			bool LoadNormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, DirectX::XMFLOAT3& outNormal);
			bool LoadBinormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, DirectX::XMFLOAT3& outBiNormal);
			bool LoadTangent(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, DirectX::XMFLOAT4& outTangent);
			void LoadSkinnedMeshInfo(FbxNode* node, JFbxSkeleton& skeleton);
			J_FBX_RESULT LoadAnimationClip(FbxScene* scene, FbxNode* node, JFbxSkeleton& skeleton, bool hasSkeleton, JFbxAnimationData& jfbxAniData);
			void LoadMaterial(FbxScene* scene);
			bool LoadTexture(FbxProperty& prop, std::string& path);	
		private:
			void SortBlendingWeight();
			void StuffSkletonData(JFbxSkeleton& fbxSkeleton, std::vector<Joint>& joint); 
			void StuffMaterial(FbxNode* node, JFbxMaterialMap& materialMap, const size_t meshGuid);
			FbxAMatrix GetGeometryTransformation(FbxNode* inNode);  
			void CheckModelAxis(JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint)noexcept;
			void GetModelYUP(std::vector<Joint>& joint, int& outUpDir, FbxAxisSystem::EUpVector& outUpAxis)noexcept;
			void SetLeftHandMatrix(FbxAMatrix& m)noexcept;
			void SetLeftHandPosition(FbxVector4& t)noexcept;
			void SetLeftHandQuaternion(FbxQuaternion& q)noexcept;
			void SetLeftHandScale(FbxVector4& s)noexcept;
			void SetRotationDegreeToRadian(FbxAMatrix& mat)noexcept;
			void SetRotationDegreeToRadian(FbxVector4& r)noexcept;
			void SetSceneAxis(FbxScene* scene, FbxNode* root)noexcept;
		private:
			void ConvertFbaToJM(FbxAMatrix& fbxM, JMatrix4x4& m)noexcept;  
			void ConvertAxis(JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept;
			void ConvertPosition(JVector3<float>& posion, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept;
			void ConvertQuaternion(JVector4<float>& posion, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept;
			void ConvertScale(JVector3<float>& posion, int modelUpDir, FbxAxisSystem::EUpVector modelUpV)noexcept;
			size_t MakeVertexMapKey(const DirectX::XMFLOAT3& positionXm,
				const DirectX::XMFLOAT2& textureXm,
				const DirectX::XMFLOAT3& normalXm,
				const DirectX::XMFLOAT3& biNormalXm,
				const DirectX::XMFLOAT4& tangentXm,
				const bool isSkin)noexcept;
			void ResizeMatrix(JMatrix4x4& m)noexcept; 
			void ResizeVertexPosition(JVector3<float>& vertexPosition)noexcept;
		};
	}
	using JFbxFileLoader = Core::JSingletonHolder<Core::JFbxFileLoader>;
}
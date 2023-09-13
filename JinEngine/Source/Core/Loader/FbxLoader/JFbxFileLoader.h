#pragma once
#include<winerror.h>
#include<fstream>
#include<algorithm>
#include"JFbxUtility.h"
#include"JFbxResult.h" 
#include"../../Singleton/JSingletonHolder.h"

namespace JinEngine
{ 
	struct JSkeleton;
	struct Joint;
	class JAnimationClip;

	namespace Core
	{
		class JFbxFileLoaderImpl
		{
		public:
			struct FbxFileTypeInfo
			{
			public:
				J_FBXRESULT typeInfo = J_FBXRESULT::FAIL;
				std::string meshRootName;
				std::string skeletonRootName;
				std::string animName;
			};
		private:
			FbxManager* fbxManager;
			std::unordered_map<size_t, uint32> vertexIndexMap;
			std::vector<JFbxControlPoint> controlPoint;
			std::vector<JBlendingIndexWeightPair> nowBlendingPair[3];
			DirectX::XMFLOAT3 positionXm[3];
			DirectX::XMFLOAT2 textureXm[3];
			DirectX::XMFLOAT3 normalXm[3];
			DirectX::XMFLOAT3 biNormalXm[3];
			DirectX::XMFLOAT4 tangentXm[3];
			uint indexingOrder[3] = { 0,1,2 };
			bool isSkinMesh[3];
			uint vertexCount = 0;
			FbxAxisSystem::EUpVector sceneUpV;
			FbxAxisSystem::EFrontVector sceneFrontV;
			FbxAxisSystem::ECoordSystem sceneCoordSys;
			FbxAxisSystem::EPreDefinedAxisSystem sceneAxis;
			FbxAxisSystem sceneAxisSystem;

			int sceneUpDir;
			int sceneFrontDir;
			const DirectX::XMFLOAT4 ZUpToYUpxMinus90{ 0.707107f, 0.0f, 0.0f,-0.707107f };
			const DirectX::XMFLOAT4X4 YUpRToYUpL
			{
				-1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,0,1
			};
			const DirectX::XMFLOAT4X4 ZUpRToYUpL
			{
				-1,0,0,0,
				0,0,-1,0,
				0,1,0,0,
				0,0,0,1
			};
			const int failUpDirSearch = -2;
			float resizeRate = 1;
		private:
			//Debug 
			int num = 0;
		public:
			~JFbxFileLoaderImpl();
			J_FBXRESULT LoadFbxMeshFile(const std::string& path, JStaticMeshGroup& meshGroup);
			J_FBXRESULT LoadFbxMeshFile(const std::string& path, JSkinnedMeshGroup& meshGroup, std::vector<Joint>& joint);
			J_FBXRESULT LoadFbxAnimationFile(const std::string& path, JFbxAnimationData& jfbxAniData);
			FbxFileTypeInfo GetFileTypeInfo(const std::string& path);
		private:
			void GetMeshCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo);
			void GetJointCount(FbxNode* node, uint& count, FbxFileTypeInfo& typeInfo);
			void LoadNode(FbxNode* node, JStaticMeshGroup& meshGroup);
			void LoadNode(FbxNode* node, JSkinnedMeshGroup& meshGroup, JFbxSkeleton& skeleton);
			void LoadJoint(FbxNode* node, int depth, int index, int parentIndex, JFbxSkeleton& skeleton);
			JStaticMeshData LoadStaticMesh(FbxNode* node);
			JSkinnedMeshData LoadSkinnedMesh(FbxNode* node, JFbxSkeleton& skeleton);
			void LoadControlPoint(FbxMesh* mesh);
			bool LoadTextureUV(const FbxMesh* mesh, int controlPointIndex, int inTextureUVIndex, DirectX::XMFLOAT2& outUV);
			bool LoadNormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, DirectX::XMFLOAT3& outNormal);
			bool LoadBinormal(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, DirectX::XMFLOAT3& outBiNormal);
			bool LoadTangent(const FbxMesh* mesh, int controlPointIndex, int vertexCounter, DirectX::XMFLOAT4& outTangent);
			void LoadSkinnedMeshInfo(FbxNode* node, JFbxSkeleton& skeleton);
			J_FBXRESULT LoadAnimationClip(FbxScene* scene, FbxNode* node, JFbxSkeleton& skeleton, bool hasSkeleton, JFbxAnimationData& jfbxAniData);
		private:
			void SortBlendingWeight(); 
			void StuffSkletonData(JFbxSkeleton& fbxSkeleton, std::vector<Joint>& joint);
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
			void ResizeVertexPosition(DirectX::XMFLOAT3& vertexPosition)noexcept;
			void ResizeVertexPosition(JVector3<float>& vertexPosition)noexcept;
		};
		using JFbxFileLoader = JSingletonHolder<JFbxFileLoaderImpl>;
	}
}
#pragma once
#include<string>
#include"../../JDataType.h"  
#include"../../../Object/Resource/Mesh/JMeshStruct.h"
#include"../../../Object/Resource/AnimationClip/JAnimationSample.h"
#include"../../../Utility/Vector.h"
#include"../../../../Lib/Fbx/Include/fbxsdk.h"
#include"../../../../Lib/DirectX/DirectXMesh.h" 

namespace JinEngine
{ 
	namespace Core
	{
		inline static bool CompareBlendingWeight(const JBlendingIndexWeightPair& a, const JBlendingIndexWeightPair& b)noexcept
		{
			return a.blendingWeight > b.blendingWeight;
		}
		struct JFbxPartMeshData
		{
		public:
			std::string name;
			int parentIndex = -1;
			JStaticMeshData staticMeshData;
			JSkinnedMeshData skinnedMeshData;
			DirectX::BoundingBox boundingBox;
			DirectX::BoundingSphere boundingSphere;
			bool hasMesh;
		};
		struct JFbxKeyFrame
		{
		public:
			FbxLongLong frameNum;
			FbxVector4 quaternion;
			FbxVector4 translation;
			FbxVector4 scale;
			JFbxKeyFrame* next = nullptr;
		};
		struct JFbxJoint
		{
		public:
			std::string name;
			JFbxKeyFrame* animation;
			FbxNode* node;
			int parentIndex;
			FbxAMatrix globalBindPositionInverse;
			Vector3<float> min;
			Vector3<float> max;
			float size;
		public:
			JFbxJoint();
			~JFbxJoint();
		};
		class JFbxSkeleton
		{
		public:
			uint jointCount = 0;
			std::vector<JFbxJoint> joint; 
		};
		struct JFbxControlPoint
		{
		public:
			Vector3<float> position;
			std::vector<JBlendingIndexWeightPair> blendingInfo;
			bool isSkin = false;
		};

		struct JFbxAnimationData
		{
		public:
			std::vector<JAnimationSample>animationSample;
			size_t skeletonHash;
			uint32 clipLength;
			float framePerSecond;
		};
	}
}
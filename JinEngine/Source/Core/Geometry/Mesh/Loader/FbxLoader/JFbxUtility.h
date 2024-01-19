#pragma once
#include<string>  
#include"../../JMeshStruct.h"
#include"../../../../Animation/JAnimationSample.h"
#include"../../../../ThirdParty/Fbx/Include/fbxsdk.h"
#include"../../../../ThirdParty/DirectX/DirectXMesh.h" 

namespace JinEngine
{ 
	namespace Core
	{
		inline static bool CompareBlendingWeight(const JBlendingIndexWeightPair& a, const JBlendingIndexWeightPair& b)noexcept
		{
			return a.blendingWeight > b.blendingWeight;
		}
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
			JVector3<float> min;
			JVector3<float> max;
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
		struct JFbxMaterial
		{
		public:
			std::string name;
		public:
			JVector4F albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };	//same as JMaterial default value
			float metallic = 0.25f;			//same as JMaterial default value
			float roughness = 0.75f;		//same as JMaterial default value
		public:
			//texture name(include path)
			//has empty value("") if not exist
			std::string albedoMapName;
			std::string normalMapName;
			std::string bumpMapName;
			std::string heightMapName;
			std::string roughnessMapName;
			std::string metalicMapName;
			std::string ambientMapName;	 

			std::string emissiveName;
			std::string specularName; 
			std::string transparentColorName;
			std::string vectorDisplacementColorName;
		};
		struct JFbxControlPoint
		{
		public:
			JVector3<float> position;
			std::vector<JBlendingIndexWeightPair> blendingInfo;
			bool isSkin = false;
		};

		struct JFbxAnimationData
		{
		public:
			std::vector<JAnimationSample>animationSample;
			size_t skeletonHash;
			uint32 length;
			float framePerSecond;
		};
	}
}
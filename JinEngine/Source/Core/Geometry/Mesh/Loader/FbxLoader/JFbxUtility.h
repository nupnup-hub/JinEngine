/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"../../JMeshStruct.h"
#include"../../JMaterialProperty.h"
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
			JMaterialParameter mParam;
			//JVector4F albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };	 
			//float metallic = 0.25f;			 
			//float roughness = 0.75f;		 
			//float specularFactor = 0.5f;	
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
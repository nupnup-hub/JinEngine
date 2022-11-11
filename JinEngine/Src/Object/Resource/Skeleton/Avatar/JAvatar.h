#pragma once
#include <unordered_map>
#include"JAvatarJointType.h"
#include"JAvatarJoint.h" 
#include"../JSkeletonFixedData.h"
#include"../../../../Core/JDataType.h"

namespace JinEngine
{
	class JAvatarEditor;
	class JSkeletonAsset;

	class JAvatar
	{
	private:
		friend class JAvatarEditor;
		friend class JSkeletonAsset;
	public:
		struct JAvatarJointReference
		{
		public:
			uint8 refIndex = JSkeletonFixedData::incorrectJointIndex;
			uint8 allottedParentRefIndex = JSkeletonFixedData::incorrectJointIndex;
		};
	public: 
		static const std::vector<std::vector<JAvatarJointGuide>> jointGuide;
		static const std::vector<std::string> tabName;
		static const std::vector<uint8> jointReferenceParent;
		static const std::unordered_map<uint8, std::vector<uint8>> jointReferenceChildren;
		static const std::unordered_map<J_AVATAR_JOINT, uint8> jointReferenceMap;
		std::vector<uint8> jointReference = std::vector<uint8>(JSkeletonFixedData::maxAvatarJointCount, JSkeletonFixedData::incorrectJointIndex);
		std::vector<JAvatarJointReference> jointBackReferenceMap;
		std::vector<JAvatarInterpolation> jointInterpolation;
	public:  
		//Find allotted joint reference Index end joint to st Joint
		uint8 FindReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed)noexcept;
		uint8 FindReferenceIndexEndToRoot(const uint8 st, const uint8 ed)noexcept;
		void FindCommonReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed, const JAvatar* target, uint8& srcIndex, uint8& tarIndex)noexcept;
	};
}
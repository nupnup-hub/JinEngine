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
#include <unordered_map>
#include"JAvatarJointType.h"
#include"JAvatarJoint.h" 
#include"../JSkeletonFixedData.h"
#include"../../../../Core/Reflection/JReflection.h"
#include"../../../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Editor
	{
		class JAvatarEditor;
	} 
	class JSkeletonAsset;
	class JAvatar
	{
		REGISTER_CLASS_USE_ALLOCATOR(JAvatar)
	private:
		friend class Editor::JAvatarEditor;
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
		static const std::vector<uint8> jointReferenceParent;	// jointReference parent index
		static const std::unordered_map<uint8, std::vector<uint8>> jointReferenceChildren;
		static const std::unordered_map<J_AVATAR_JOINT, uint8> jointReferenceMap;
	private:
		JWeakPtr<JSkeletonAsset> ownerSkeleton;
	public:
		//value is skeleton joint index
		//index is defined avatar joint index
		std::vector<uint8> jointReference = std::vector<uint8>(JSkeletonFixedData::maxAvatarJointCount, JSkeletonFixedData::incorrectJointIndex);
		std::vector<JAvatarJointReference> jointBackReferenceMap;
		std::vector<JAvatarInterpolation> jointInterpolation;
	public:  
		//Find allotted joint reference Index end joint to st Joint
		uint8 FindReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed)noexcept;
		uint8 FindReferenceIndexEndToRoot(const uint8 st, const uint8 ed)noexcept;
		void FindCommonReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed, const JAvatar* target, uint8& srcIndex, uint8& tarIndex)noexcept;
	private: 
		JAvatar() = default;	//for avatar editor
		~JAvatar() = default;
		JAvatar(const JAvatar& rhs) = delete;
		JAvatar(JAvatar&& rhs) = delete;
		JAvatar operator=(const JAvatar& rhs) = delete;
		JAvatar operator=(JAvatar&& rhs) = delete;
	};
}
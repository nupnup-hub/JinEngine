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


#include"JAvatar.h"

namespace JinEngine
{
	struct JointReferenceInfo
	{
	public:
		JAvatarJointGuide guide;
		std::string categoryName;
		uint8 parentIndex;
		std::vector<uint8> childIndex;
	public:
		JointReferenceInfo(const JAvatarJointGuide& guide, const std::string& categoryName, const uint8 parentIndex)
			:guide(guide), categoryName(categoryName), parentIndex(parentIndex)
		{}
		JointReferenceInfo(const JAvatarJointGuide& guide, const std::string& categoryName, const uint8 parentIndex, std::vector<uint8> childIndex)
			:guide(guide), categoryName(categoryName), parentIndex(parentIndex), childIndex(childIndex)
		{}
	};
	static std::vector<JointReferenceInfo> GetJointReferenceInfo()
	{
		static std::vector<JointReferenceInfo> info;
		if (info.size() == 0)
		{
			info.push_back(JointReferenceInfo({ "Root", "root", 0 }, "Body", 255, { 1 }));
			info.push_back(JointReferenceInfo({ "Hips", "pervis", 1 }, "Body", 0, { 2, 47, 53 }));
			info.push_back(JointReferenceInfo({ "Spine", "spine_01", 2 }, "Body", 1, { 3 }));
			info.push_back(JointReferenceInfo({ "Chest", "spine_02", 3 }, "Body", 2, { 4 }));
			info.push_back(JointReferenceInfo({ "Upper Chest", "spine_03", 4 }, "Body", 3, { 5, 26, 59 }));
			info.push_back(JointReferenceInfo({ "Left Shoulder", "clavicle_l", 5 }, "Body", 4, { 6 }));
			info.push_back(JointReferenceInfo({ "Left Upper Arm", "upperarm_l", 6 }, "Body", 5, { 7, 25 }));
			info.push_back(JointReferenceInfo({ "Left Lower Arm", "lowerarm_l", 7 }, "Body", 6, { 8, 24 }));
			info.push_back(JointReferenceInfo({ "Left Hand", "hand_l", 8 }, "Body", 7, { 9, 12, 15, 18, 21 }));
			info.push_back(JointReferenceInfo({ "Left Index Proximal", "index_01_l", 9 }, "Left Hand", 8, { 10 }));
			info.push_back(JointReferenceInfo({ "Left Index Intermediate", "index_02_l", 10 }, "Left Hand", 9, { 11 }));
			info.push_back(JointReferenceInfo({ "Left Index Distal", "index_03_l", 11 }, "Left Hand", 10, {}));
			info.push_back(JointReferenceInfo({ "Left Middle Proximal", "middle_01_l", 12 }, "Left Hand", 8, { 13 }));
			info.push_back(JointReferenceInfo({ "Left Middle Intermediate", "middle_02_l", 13 }, "Left Hand", 12, { 14 }));
			info.push_back(JointReferenceInfo({ "Left Middle Distal", "middle_03_l", 14 }, "Left Hand", 13, {}));
			info.push_back(JointReferenceInfo({ "Left Little Proximal", "pinky_01_l", 15 }, "Left Hand", 8, { 16 }));
			info.push_back(JointReferenceInfo({ "Left Little Intermediate", "pinky_02_l", 16 }, "Left Hand", 15, { 17 }));
			info.push_back(JointReferenceInfo({ "Left Little Distal", "pinky_03_l", 17 }, "Left Hand", 16, {}));
			info.push_back(JointReferenceInfo({ "Left Ring Proximal", "ring_01_l", 18 }, "Left Hand", 8, { 19 }));
			info.push_back(JointReferenceInfo({ "Left Ring Intermediate", "ring_02_l", 19 }, "Left Hand", 18, { 20 }));
			info.push_back(JointReferenceInfo({ "Left Ring Distal", "ring_03_l", 20 }, "Left Hand", 19, {}));
			info.push_back(JointReferenceInfo({ "Left Thumb Proximal", "thumb_01_l", 21 }, "Left Hand", 8, { 22 }));
			info.push_back(JointReferenceInfo({ "Left Thumb Intermediate", "thumb_02_l", 22 }, "Left Hand", 21, { 23 }));
			info.push_back(JointReferenceInfo({ "Left Thumb Distal", "thumb_03_l", 23 }, "Left Hand", 22, {}));
			info.push_back(JointReferenceInfo({ "Left Lower Arm Twist", "lowerarm_twist_01_l", 24 }, "Body", 7, {}));
			info.push_back(JointReferenceInfo({ "Left Upper Arm Twist", "upperarm_twist_01_l", 25 }, "Body", 6, {}));
			info.push_back(JointReferenceInfo({ "Right Shoulder", "clavicle_r", 26 }, "Body", 4, { 27 }));
			info.push_back(JointReferenceInfo({ "Right Upper Arm", "upperarm_r", 27 }, "Body", 26, { 28, 46 }));
			info.push_back(JointReferenceInfo({ "Right Lower Arm", "lowerarm_r", 28 }, "Body", 27, { 29, 45 }));
			info.push_back(JointReferenceInfo({ "Right Hand", "hand_r", 29 }, "Body", 28, { 30, 33, 36, 39, 42 }));
			info.push_back(JointReferenceInfo({ "Right Index Proximal", "index_01_r", 30 }, "Right Hand", 29, { 31 }));
			info.push_back(JointReferenceInfo({ "Right Index Intermediate", "index_02_r", 31 }, "Right Hand", 30, { 32 }));
			info.push_back(JointReferenceInfo({ "Right Index Distal", "index_03_r", 32 }, "Right Hand", 31, {}));
			info.push_back(JointReferenceInfo({ "Right Middle Proximal", "middle_01_r", 33 }, "Right Hand", 29, { 34 }));
			info.push_back(JointReferenceInfo({ "Right Middle Intermediate", "middle_02_r", 34 }, "Right Hand", 33, { 35 }));
			info.push_back(JointReferenceInfo({ "Right Middle Distal", "middle_03_r", 35 }, "Right Hand", 34, {}));
			info.push_back(JointReferenceInfo({ "Right Little Proximal", "pinky_01_r", 36 }, "Right Hand", 29, { 37 }));
			info.push_back(JointReferenceInfo({ "Right Little Intermediate", "pinky_02_r", 37 }, "Right Hand", 36, { 38 }));
			info.push_back(JointReferenceInfo({ "Right Little Distal", "pinky_03_r", 38 }, "Right Hand", 37, {}));
			info.push_back(JointReferenceInfo({ "Right Ring Proximal", "ring_01_r", 39 }, "Right Hand", 29, { 40 }));
			info.push_back(JointReferenceInfo({ "Right Ring Intermediate", "ring_02_r", 40 }, "Right Hand", 39, { 41 }));
			info.push_back(JointReferenceInfo({ "Right Ring Distal", "ring_03_r", 41 }, "Right Hand", 40, {}));
			info.push_back(JointReferenceInfo({ "Right Thumb Proximal", "thumb_01_r", 42 }, "Right Hand", 29, { 43 }));
			info.push_back(JointReferenceInfo({ "Right Thumb Intermediate", "thumb_02_r", 43 }, "Right Hand", 42, { 44 }));
			info.push_back(JointReferenceInfo({ "Right Thumb Distal", "thumb_03_r", 44 }, "Right Hand", 43, {}));
			info.push_back(JointReferenceInfo({ "Right Lower Arm Twist", "lowerarm_twist_01_r", 45 }, "Body", 28, {}));
			info.push_back(JointReferenceInfo({ "Right Upper Arm Twist", "upperarm_twist_01_r", 46 }, "Body", 27, {}));
			info.push_back(JointReferenceInfo({ "Left Upper Leg", "thigh_l", 47 }, "Body", 1, { 48, 52 }));
			info.push_back(JointReferenceInfo({ "Left Lower Leg", "calf_l", 48 }, "Body", 47, { 49, 50 }));
			info.push_back(JointReferenceInfo({ "Left Calf Twist", "calf_twist_01_l", 49 }, "Body", 48, {}));
			info.push_back(JointReferenceInfo({ "Left Foot", "foot_l", 50 }, "Body", 48, { 51 }));
			info.push_back(JointReferenceInfo({ "Left Toes", "ball_l", 51 }, "Body", 50, {}));
			info.push_back(JointReferenceInfo({ "Left Thigh Twist", "thigh_twist_01_l", 52 }, "Body", 47, {}));
			info.push_back(JointReferenceInfo({ "Right Upper Leg", "thigh_r", 53 }, "Body", 1, { 54, 58 }));
			info.push_back(JointReferenceInfo({ "Right Lower Leg", "calf_r", 54 }, "Body", 53, { 55, 56 }));
			info.push_back(JointReferenceInfo({ "Right Calf Twist", "calf_twist_01_r", 55 }, "Body", 54, {}));
			info.push_back(JointReferenceInfo({ "Right Foot", "foot_r", 56 }, "Body", 54, { 57 }));
			info.push_back(JointReferenceInfo({ "Right Toes", "ball_r", 57 }, "Body", 56, {}));
			info.push_back(JointReferenceInfo({ "Right Thigh Twist", "thigh_twist_01_r", 58 }, "Body", 53, {}));
			info.push_back(JointReferenceInfo({ "Neck", "neck_01", 59 }, "Head", 4, { 60 }));
			info.push_back(JointReferenceInfo({ "Head", "head", 60 }, "Head", 59, { 61, 62, 63 }));
			info.push_back(JointReferenceInfo({ "Left Eye", "Eye_l", 61 }, "Head", 60, {}));
			info.push_back(JointReferenceInfo({ "Right Eye", "Eye_r", 62 }, "Head", 60, {}));
			info.push_back(JointReferenceInfo({ "Jaw", "Mouth", 63 }, "Head", 60, {}));
		}
		return info;
	}
 
	const std::vector<std::vector<JAvatarJointGuide>>& JAvatar::GetAllJointGuide()
	{
		static std::vector<std::vector<JAvatarJointGuide>> guide;
		if (guide.size() == 0)
		{
			auto info = GetJointReferenceInfo();
			guide.resize(jointCategoryCount);

			for (uint i = 0; i < jointCategoryCount; ++i)
			{
				const std::string categoryName = GetJointCategoryName(i);
				for (uint j = 0; j < info.size(); ++j)
				{
					if (info[j].categoryName == categoryName)
						guide[i].push_back(info[j].guide);
				}
			}
		}
		return guide;
	}
	const std::vector<JAvatarJointGuide>& JAvatar::GetJointCategoryGuide(const uint index)
	{
		return GetAllJointGuide()[index];
	}
	std::string JAvatar::GetJointCategoryName(const uint index)
	{
		static const std::string tabName[jointCategoryCount]
		{
			"Body", "Head", "Left Hand", "Right Hand"
		};
		return index > jointCategoryCount ? tabName[index] : "invalid index";
	}
	uint8 JAvatar::GetJointReferenceIndex(const J_AVATAR_JOINT joint)
	{
		static std::vector<uint8> jointReferenceVec;
		if (jointReferenceVec.size() == 0)
		{
			const uint jointCount = (uint)J_AVATAR_JOINT::COUNT;
			auto info = GetJointReferenceInfo();
			jointReferenceVec.resize(jointCount); 

			for (uint i = 0; i < jointCount; ++i)
				jointReferenceVec[i] = info[i].guide.index;
		}
		return jointReferenceVec[(uint)joint];
	}
	uint8 JAvatar::GetJointReferenceParent(const uint8 index)
	{
		static std::vector<uint8> jointReferenceParent;
		if (jointReferenceParent.size() == 0)
		{
			const uint jointCount = (uint)J_AVATAR_JOINT::COUNT;
			auto info = GetJointReferenceInfo();
			jointReferenceParent.resize(jointCount);

			for (uint i = 0; i < jointCount; ++i)
				jointReferenceParent[i] = info[i].parentIndex;
		}
		return jointReferenceParent[index];
	}
	const std::vector<uint8>& JAvatar::GetJointReferenceChildren(const uint8 index)
	{
		static std::vector<std::vector<uint8>> jointReferenceChildren;
		if (jointReferenceChildren.size() == 0)
		{
			const uint jointCount = (uint)J_AVATAR_JOINT::COUNT;
			auto info = GetJointReferenceInfo();
			jointReferenceChildren.resize(jointCount);

			for (uint i = 0; i < jointCount; ++i)
				jointReferenceChildren[i] = info[i].childIndex;
		}
		return jointReferenceChildren[index];
	}
	uint8 JAvatar::FindReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed)noexcept
	{
		if (st < ed)
			return JSkeletonFixedData::incorrectJointIndex;

		uint8 stIndex = GetJointReferenceIndex(st);
		if (jointReference[stIndex] != JSkeletonFixedData::incorrectJointIndex)
			return stIndex;

		uint8 edIndex = GetJointReferenceIndex(ed);
		if (jointReference[edIndex] == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;

		uint nowJointRefIndex = GetJointReferenceParent(stIndex);
		while (nowJointRefIndex != edIndex && nowJointRefIndex != JSkeletonFixedData::incorrectJointIndex)
		{
			if (jointReference[nowJointRefIndex] != JSkeletonFixedData::incorrectJointIndex)
				return nowJointRefIndex;

			nowJointRefIndex = GetJointReferenceParent(nowJointRefIndex);
		}

		if (nowJointRefIndex == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;
		else
			return edIndex;
	}
	uint8 JAvatar::FindReferenceIndexEndToRoot(const uint8 st, const uint8 ed)noexcept
	{
		if (jointReference[st] != JSkeletonFixedData::incorrectJointIndex)
			return st;

		if (jointReference[ed] == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;

		uint nowJointRefIndex = GetJointReferenceParent(st);
		while (nowJointRefIndex != ed && nowJointRefIndex != JSkeletonFixedData::incorrectJointIndex)
		{
			if (jointReference[nowJointRefIndex] != JSkeletonFixedData::incorrectJointIndex)
				return nowJointRefIndex;

			nowJointRefIndex = GetJointReferenceParent(nowJointRefIndex);
		}

		if (nowJointRefIndex == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;
		else
			return nowJointRefIndex;
	}
	void JAvatar::FindCommonReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed, const JAvatar* target, uint8& srcIndex, uint8& tarIndex)noexcept
	{
		srcIndex = FindReferenceIndexEndToRoot(st, ed);
		tarIndex = FindReferenceIndexEndToRoot(st, ed);

		if (srcIndex > tarIndex)
			srcIndex = tarIndex;
		else if (srcIndex < tarIndex)
			tarIndex = srcIndex;
	}
}
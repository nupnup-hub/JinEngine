#pragma once
#include"../../../Core/JDataType.h" 

namespace JinEngine
{
	class JSkeletonFixedData
	{
	public:
		static constexpr uint maxJointCount = 256;
		static constexpr uint8 rootJointIndex = 0;
		static constexpr uint8 incorrectJointIndex = 255;
		static constexpr uint8 maxAvatarJointCount = 64;
		static constexpr uint8 avatarBoneIndexCount = 61;
	};
}
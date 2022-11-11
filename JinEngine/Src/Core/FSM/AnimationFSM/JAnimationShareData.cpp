#include"JAnimationShareData.h"
#include"../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		void JAnimationShareData::Initialize()noexcept
		{
			for (uint i = 0; i < 2; ++i)
			{
				if (localTransform[i].size() == 0)
				{
					localTransform[i].reserve(JSkeletonFixedData::maxJointCount);
					localTransform[i].resize(JSkeletonFixedData::maxJointCount);
				}
			}
			for (uint i = 0; i < 4; ++i)
			{
				if (ikJoint[i].size() == 0)
				{
					ikJoint[i].reserve(JAnimationFixedData::ikVectorSize);
					ikJoint[i].resize(JAnimationFixedData::ikVectorSize);
				}
			}
			if (ikRate.size() == 0)
			{
				ikRate.reserve(JSkeletonFixedData::maxJointCount);
				ikRate.resize(JSkeletonFixedData::maxJointCount);
			}
		}
		void JAnimationShareData::ClearSkeletonBlendRate(const uint index)noexcept
		{
			if (index >= JAnimationFixedData::defaultLocalTransforCount)
				return;

			for (auto& data : skeletonBlendRate[index])
				data.second = 0;
		}
		bool JAnimationShareData::FindClipGuid(size_t& guid, const uint index)noexcept
		{
			for (auto& data : skeletonBlendRate[index])
			{
				if (data.second == 1)
				{
					guid = data.first;
					return true;
				}
			}
			return false;
		}
		void JAnimationShareData::SetIKRate(const uint ikNumber)noexcept
		{
			for (uint i = 0; i < ikCount[ikNumber]; ++i)
			{
				uint8 jointIndex = ikJoint[ikNumber][i].jointIndex;
				++ikRate[jointIndex].count;
				ikRate[jointIndex].rate = (float)(1.0f / ikRate[jointIndex].count);
			}
		}
		void JAnimationShareData::EnterCalculateIK()noexcept
		{
			for (uint i = 0; i < ikRate.size(); ++i)
			{
				ikRate[i].count = 0;
				ikRate[i].rate = 0;
			}

			for (uint i = 0; i < 4; ++i)
				ikCount[i] = 0;
		}
		void JAnimationShareData::StuffIdentity(const uint localIndex)noexcept
		{
			const XMFLOAT4X4 iden = JMathHelper::Identity4x4();
			for (uint i = 0; i < JSkeletonFixedData::maxJointCount; ++i)
				localTransform[localIndex][i] = iden;
		}
	}
}
#include"JAnimationConstants.h" 

namespace JinEngine
{
	namespace Graphic
	{
		void JAnimationConstants::StuffIdentity()noexcept
		{
			const JMatrix4x4 iden = JMatrix4x4::Identity();
			for (uint i = 0; i < JSkeletonFixedData::maxJointCount; ++i)
				boneTransforms[i] = iden;
		}
	}
}
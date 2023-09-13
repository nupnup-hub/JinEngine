#pragma once 
#include"../../Core/Math/JMatrix.h"
#include"../../Object/Resource/Skeleton/JSkeletonFixedData.h"

namespace JinEngine
{
    namespace Graphic
    {
        struct JAnimationConstants
        {
        public:
            JMatrix4x4 boneTransforms[JSkeletonFixedData::maxJointCount];
        public:
            void StuffIdentity()noexcept;
        };
    }
}
#pragma once
#include"JOcclusionConstants.h" 
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		template<uint setCount>
		struct JFrameConstantsSet
		{ 
		public:
			uint frameIndex[setCount];
		public:
			bool isUpdated[setCount];
			bool updateStart = false;
		public:
			virtual ~JFrameConstantsSet() = default;
		public:
			virtual void Begin()
			{
				JCUtil::InsertValue(isUpdated, false, std::make_index_sequence<setCount>());
				updateStart = false;
			}
		};
	}
}
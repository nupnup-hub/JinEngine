#pragma once
#include"JOcclusionConstants.h" 
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		/**
		* Caution!
		* Constants set은 256 byte단위로 Gpu에 upload되며
		* 이는 Cpu Constants structure size와 다를수있으므로
		* 복수의 data를 CopyData할시에는 Gpu에 upload되는 크기와 같은
		* structure을 할당하거나(권장), 하나씩 CopyData를 해야한다(비권장 너무느림)
		*/
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
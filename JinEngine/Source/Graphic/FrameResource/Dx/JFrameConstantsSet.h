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
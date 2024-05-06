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
#include"JGraphicResourceType.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../../Core/Math/JVector.h" 
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceHolder : public JGraphicDeviceUser
		{
		public:
			virtual ~JGraphicResourceHolder() = default;
		public: 
			virtual uint GetWidth()const noexcept = 0;
			virtual uint GetHeight()const noexcept = 0;
			virtual uint GetArrayCount()const noexcept = 0;
			virtual uint GetElementCount()const noexcept = 0;
			virtual uint GetElementSize()const noexcept = 0; 
			virtual J_GRAPHIC_RESOURCE_FORMAT GetEngineFormat()const noexcept = 0;
			virtual JVector2<uint> GetResourceSize()const noexcept = 0;
		public: 
			virtual bool HasValidResource()const noexcept = 0;
		public:
			virtual void Clear() = 0;
		};
	}
}
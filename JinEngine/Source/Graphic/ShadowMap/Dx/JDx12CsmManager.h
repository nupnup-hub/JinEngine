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
#include"../JCsmManager.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12CsmManager final: public JCsmManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12CsmManager)
		public:
			~JDx12CsmManager();
		public: 
			void Initialize(JGraphicDevice* device) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			JOwnerPtr<JCsmHandlerInfo> _CreateHandler(JCsmHandleCreationDesc& desc, JCsmAreaInfo* areInfo) final;
			JOwnerPtr<JCsmTargetInfo> _CreateTarget(JCsmTargetCreationDesc& desc, JCsmAreaInfo* areInfo)final;
		private:
			void BuildResource(JGraphicDevice* device);
			void ClearResource();
		public:
			static void RegisterTypeData();
		};
	}
}

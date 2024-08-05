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
#include"JCsmType.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Object/GraphicRule/Csm/JGraphicModuleCsmUserAccess.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JCsmTargetInfo;   
		class JCsmTargetInterface  : public JCsmTargetUserInterface
		{  
			JUserPtr<JCsmTargetInfo> info;
		public:
			virtual ~JCsmTargetInterface() = default;
		public:
			//managed by JCsmManager
			//2024-08-01 ¼öÁ¤ protected -> public 
			void AddCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& newInfo);
			void RemoveCsmTargetInfo(); 
		public:
			/**
			* @brief aligned by registed time
			* @return target index in Csm resource handler if info == nullptr return -1
			*/
			int GetTargetIndex()const noexcept final; 
			DirectX::BoundingFrustum GetBoundingFrustum()const noexcept final;
		private:
			JUserPtr<JCsmTargetInfo> GetTargetInfo()const noexcept; 
		public:
			bool HasTargetInfo()const noexcept final;
		}; 
	}
}
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
#include"JCsmHandlerInfo.h"
#include"../../Object/GraphicRule/Csm/JGraphicModuleCsmUserAccess.h"  
#include"../../Core/Geometry/JBBox.h" 

namespace JinEngine
{
	namespace Graphic
	{  
		class JCsmHandlerInterface : public JCsmHandleUserInterface
		{ 
		private:  
			JUserPtr<JCsmHandlerInfo> info;
		public: 
			void AddInfo(const JUserPtr<JCsmHandlerInfo>& newInfo);
			JCsmHandlerInfo* Release();
		public:
			JCsmOption GetOption()const noexcept final;
			uint GetTargetCount()const noexcept final; 
			const JCsmComputeResult& GetComputeResult(const uint index)const noexcept final;
			JCsmHandlerInfo* GetInfo()const noexcept;
		public:
			void SetOption(const JCsmOption& newOption)noexcept;   
		public: 
			void Update(const DirectX::XMMATRIX lightView,
				const DirectX::BoundingBox& sceneBBoxW,			//world bbox 
				const size_t mapSize); 
		}; 
	}
}
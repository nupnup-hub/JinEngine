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
#include"JOccCulling.h"
#include"../../Shader/JShaderDataHandler.h"
#include"../../JGraphicSubClassInterface.h" 

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo;
		struct JGraphicOption;
		class JCullingInfo;

		/**
		* Hzb는 IsOccluder trigger가 켜진 object에 가려진 대상을 Occlude한다.
		*/
		class JHZBOccCulling : public JOccCulling, public JGraphicSubClassInterface
		{ 
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			virtual bool CanReadBackDebugInfo()const noexcept = 0; 
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void NotifyBuildNewHzbOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo) = 0;
			virtual void NotifyReBuildHzbOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo) = 0;
			virtual void NotifyDestroyHzbOccBuffer(JCullingInfo* cullingInfo) = 0;
		public:
			//Set upload buffer and shader macro
			virtual void ReBuildOcclusionMapInfoConstants(JGraphicDevice* device, const JGraphicInfo& info) = 0;	//미적용 occlusion map option 설정하는 인터페이스 추가필요
		public:
			virtual void StreamOutDebugInfo(const JUserPtr<JCullingInfo>& cullingInfo, const std::wstring& path) = 0;
			virtual void StreamOutDebugInfo(const std::wstring& path) = 0;
		public:
			virtual void DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) = 0;
			virtual void DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) = 0;
		public:
			virtual void ComputeOcclusionCulling(const JGraphicHzbOccComputeSet* computeSet, const JDrawHelper& helper) = 0;
		};
	}
}
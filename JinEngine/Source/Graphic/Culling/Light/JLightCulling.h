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
#include"../../Shader/JShaderDataHandler.h"
#include"../../JGraphicTaskInterface.h" 
#include"../../JGraphicSubClassInterface.h"
#include"../../Thread/JGraphicMultiThreadDrawInterface.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		class JDrawHelper;
		class JLightCulling : public JGraphicTaskInterface,
			public JGraphicSubClassInterface,
			public JGraphicDeviceShaderHandler
		{
		public: 
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;  
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void NotifyNewClusterOption(JGraphicDevice* device) = 0;
			virtual void NotifyLocalLightCapacityChanged(JGraphicDevice* device, JGraphicResourceManager* gM, const size_t capacity) = 0;
		public:
			virtual void BindDrawResource(const JGraphicBindSet* bindSet) = 0; 
			virtual void BindDebugResource(const JGraphicBindSet* bindSet) = 0;
		public:
			virtual void ExecuteLightClusterTask(const JGraphicLightCullingTaskSet* taskSet, const JDrawHelper& helper) = 0;
			virtual void ExecuteLightClusterDebug(const JGraphicLightCullingDebugDrawSet* drawSet, const JDrawHelper& helper) = 0;
		public: 
			virtual void StreamOutDebugInfo(const std::wstring& path) = 0;
		};
	}
}
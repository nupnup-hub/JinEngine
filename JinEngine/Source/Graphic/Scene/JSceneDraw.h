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
#include"../JGraphicTaskInterface.h"
#include"../JGraphicSubClassInterface.h"
#include"../Shader/JShaderDataHandler.h"
#include"../Thread/JGraphicMultiThreadDrawInterface.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Pointer/JOwnerPtr.h"  

namespace JinEngine
{
	class JGameObject;
	namespace Graphic
	{
		class JDrawHelper; 
		struct JGraphicInfo; 

		class JSceneDraw : public JGraphicTaskInterface,
			public JGraphicSubClassInterface,
			public JGraphicShaderDataHandler,
			public JGraphicMultiThreadDrawInterface
		{  
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0; 
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void BindResource(const J_GRAPHIC_RENDERING_PROCESS process, const JGraphicBindSet* bindSet) = 0;
		public:
			//Contain Draw Debug UI
			virtual void DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet,const JDrawHelper& helper) = 0;
			virtual void DrawSceneDebugUIMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneShade(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawSceneShadeMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
		public: 
			virtual void ComputeSceneDependencyTemporalResource(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper) = 0;
		};
	}
}
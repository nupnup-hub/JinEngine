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
#include"../../Core/Pointer/JOwnerPtr.h"  

namespace JinEngine
{
	//struct JGraphicShaderData;
	class JGameObject;
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JDrawHelper;
		struct JDrawCondition;

		class JDepthTest : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			enum class TEST_TYPE
			{
				BOUNDING_TEST = 0,
				QUERY_TEST,
				QUERY_TEST_PASS,
				COUNT
			}; 
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void DrawSceneBoundingBox(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition) = 0;
			virtual void DrawHdOcclusionQueryObject(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition) = 0;
		public:
			virtual bool BindGraphicResource(const JGraphicBindSet* bindSet, const JDrawHelper& helper) = 0;
		};
	}
}
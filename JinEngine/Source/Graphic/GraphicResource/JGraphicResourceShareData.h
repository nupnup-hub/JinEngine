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
#include"../../Core/JCoreEssential.h"
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		class JGraphicResourceManager;
		class JGraphicResourceInfo;

		class JShareDataHolderInterface : public JGraphicTaskInterface
		{ 
		public:
			virtual void UpdateBegin() = 0;
			virtual void UpdateEnd() = 0;
		};

		//shared data by shader pass
		//ex) ssao, imageProcessing(bloom, toneMapping, aa...)
		//share data is created by use resource creation
		//ex) creation ssao -> try create related resource(interleave, depth, intermediate ...)
		class JGraphicResourceShareData : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
		public: 
			virtual void Clear() = 0;
		public:  
			virtual JShareDataHolderInterface* GetResourceDependencyData(const J_GRAPHIC_TASK_TYPE taskType, JGraphicResourceInfo* info) = 0;
		public:
			virtual void NotifyGraphicResourceCreation(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newInfo) = 0;
			virtual void NotifyGraphicResourceDestruction(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* info) = 0;
		};
	}
}
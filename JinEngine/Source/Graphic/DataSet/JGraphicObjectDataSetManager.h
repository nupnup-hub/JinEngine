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
#include"JGraphicObjectDataSet.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../JGraphicSubClassInterface.h" 
#include"../../Core/JCoreEssential.h" "
#include"../../Core/Pointer/JOwnerPtr.h" 
#include"../../Object/Component/JComponentType.h"
#include"../../Object/Resource/JResourceObjectType.h"
#include"../../Object/GraphicRule/GraphicResource/JGraphicModuleTextureResourceType.h"
#include"../../Object/JObjectTypeStatistics.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		class JGraphicResourceInfo;
		class JGraphicObjectDataSetManager : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JGraphicObjectDataSetManager)
		public:
			using DataVec = ObjectDataSetVec;
		public:
			virtual void Initialize(JGraphicDevice* device) = 0;
			virtual void Clear() = 0;
		public:   
			virtual const DataVec& GetDataVec(const UniqueIndex index)const noexcept = 0;
			virtual JObjectDataSetMetadata GetMetadata(const UniqueIndex index)const noexcept = 0; 
		public:
			virtual JFastPtr<JGraphicModuleManagedDataFrame> Add(const JGraphicModuleManagedDataCreationDesc& creationData) = 0;
			virtual bool Remove(JFastPtr<JGraphicModuleManagedDataFrame>& data) = 0;
		public:
			virtual void NotifyGraphicResourceCreation(JGraphicObjectDataSetBase* base, const JUserPtr<JGraphicResourceInfo>& newInfo, const J_GRAPHIC_TASK_TYPE task) = 0;
		};
	}
}

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
#include"../JGraphicObjectDataSetManager.h"
#include"JDx12GraphicObjectDataSet.h"
#include"JDx12GraphicTaskDataSet.h"

namespace JinEngine
{ 
	namespace Graphic
	{
		class JDx12GraphicObjectDataSetManager : public JGraphicObjectDataSetManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12GraphicObjectDataSetManager)
		public:
			using DataVec = JGraphicObjectDataSetManager::DataVec; 
		private:
			DataVec set[totalCompAndResourceVariation];
			JObjectDataSetMetadata metadata[totalCompAndResourceVariation];
		public:
			void Initialize(JGraphicDevice* device) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final; 
			const DataVec& GetDataVec(const UniqueIndex index)const noexcept final;
			JObjectDataSetMetadata GetMetadata(const UniqueIndex index)const noexcept final;  
		private:
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final; 
		public:
			JFastPtr<JGraphicModuleManagedDataFrame> Add(const JGraphicModuleManagedDataCreationDesc& creationData) final;
			bool Remove(JFastPtr<JGraphicModuleManagedDataFrame>& data)final;
		private:
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
			void NotifyGraphicResourceCreation(JGraphicObjectDataSetBase* base, const JUserPtr<JGraphicResourceInfo>& newInfo, const J_GRAPHIC_TASK_TYPE task)final;
		private:
			void BuildResource(JGraphicDevice* device);
			void CreateMetadata();
			void ClearResource();   
		};
	}
}
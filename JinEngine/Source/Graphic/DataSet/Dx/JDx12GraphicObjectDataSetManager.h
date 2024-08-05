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

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicObjectDataSetManager : public JGraphicObjectDataSetManager
		{
		public: 
			enum class TYPE_PER_INDEX
			{
				COMP_BEHAVIOR = 0, 
				COMP_END = (uint)J_COMPONENT_TYPE::COUNT - 1,
				RESOURCE_BEGIN = (uint)J_COMPONENT_TYPE::COUNT,
				RESOURCE_END = RESOURCE_BEGIN + (uint)J_RESOURCE_TYPE::COUNT - 1,
				COUNT
			}; 
		public:
			using DataVec = JGraphicObjectDataSetManager::DataVec;
		private:
			DataVec set[(uint)TYPE_PER_INDEX::COUNT];
			JObjectDataSetMetadata metadata[(uint)TYPE_PER_INDEX::COUNT];
		public:
			void Initialize(JGraphicDevice* device) final;
			void Clear() final;
		public: 
			const DataVec& GetDataVec(const J_COMPONENT_TYPE type)const noexcept final;
			const DataVec& GetDataVec(const J_RESOURCE_TYPE type)const noexcept final;
			JObjectDataSetMetadata GetMetadata(const J_COMPONENT_TYPE type)const noexcept final;
			JObjectDataSetMetadata GetMetadata(const J_RESOURCE_TYPE type)const noexcept final;
		public:
			bool Add(const JUserPtr<JObject>& obj) final;
			bool Remove(JUserPtr<JGraphicModuleManagedDataFrame>& data)final;
		private:
			void BuildResource(JGraphicDevice* device);
			void CreateMetadata();
			void ClearResource();  
		};
	}
}
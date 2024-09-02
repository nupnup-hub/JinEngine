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
#include"JGraphicModuleTextureResourceType.h"
#include"../JGraphicModuleType.h"
#include"../../../Core/Handle/JDataHandleStructure.h"

namespace JinEngine
{
	class JGraphicResourceUserInterface
	{ 
	public:
		JGraphicResourceUserInterface() = default; 
		virtual ~JGraphicResourceUserInterface() = default;
	public:
		virtual uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;  
		virtual uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
		virtual uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
		virtual JVector2F GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
		virtual JVector2F GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
		virtual int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
		virtual int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual int GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept = 0;
		virtual int GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept = 0;
		virtual uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept = 0;
		virtual uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual uint GetMipmapCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept = 0;
		virtual ResourceHandle GetOptionGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex, const uint dataIndex) const noexcept = 0;
	public:
		virtual int GetFirstResourceArrayIndex()const noexcept = 0;
		virtual int GetFirstResourceHeapStart(const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
		virtual JVector2F GetFirstResourceSize()const noexcept = 0;
		virtual JVector2F GetFirstResourceInvSize()const noexcept = 0;
		/**
		* @return invalid index(cast J_GRAPHIC_RESOURCE_TYPE) if access invalid data
		*/
		virtual J_GRAPHIC_RESOURCE_TYPE GetFirstResourceType()const noexcept = 0;
		virtual ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept = 0;
		virtual J_GRAPHIC_MIP_MAP_TYPE GetFirstMipmapType()const noexcept = 0; 
		/**
		* @brief return resource data Index(single) or offset(multi)
		* @brief basically dataIndex defined by task type
		* @return invalid index if not supported type
		*/
		virtual int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)const noexcept = 0;
	public:
		virtual bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
		virtual bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual bool HasFirstHandle()const noexcept = 0;
		virtual bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
		virtual bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept = 0;
		virtual bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
	};
}

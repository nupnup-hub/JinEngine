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
#include"JGraphicModuleCullingType.h"

namespace JinEngine
{
	class JCullingUserInterface
	{ 
	public:
		JCullingUserInterface() = default;
		virtual ~JCullingUserInterface() = default;
	public:
		virtual int GetArrayIndex(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;
		virtual uint GetResultBufferSize(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;
		virtual float GetUpdateFrequency(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;
	public:
		virtual void SetCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept = 0;
		virtual void OffCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept = 0;
		virtual void OffCullingArray(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept = 0;
	public:
		virtual bool IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept = 0;	//culling array is ordered by frame index
		virtual bool IsCulled(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)const noexcept = 0;
		virtual bool IsUpdateEnd(const J_CULLING_TYPE type)const noexcept = 0;
		virtual bool IsUpdateEnd(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;
		virtual bool HasCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;												//culling array is ordered by render item mesh number
	};
}

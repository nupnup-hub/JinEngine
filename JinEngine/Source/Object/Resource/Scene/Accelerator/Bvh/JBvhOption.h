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
#include"../JAcceleratorType.h"
#include"../JAcceleratorOption.h"
namespace JinEngine
{
	struct JBvhOption
	{
	public:
		J_ACCELERATOR_BUILD_TYPE buildType = J_ACCELERATOR_BUILD_TYPE::TOP_DOWN;
		J_ACCELERATOR_SPLIT_TYPE splitType = J_ACCELERATOR_SPLIT_TYPE::SAH;
	public:
		JAcceleratorOption commonOption;
	public:
		JBvhOption() = default;
		JBvhOption(const J_ACCELERATOR_BUILD_TYPE buildType, const J_ACCELERATOR_SPLIT_TYPE splitType, const JAcceleratorOption& commonOption);
	public:
		bool EqualBvhOption(const JBvhOption& tar)const noexcept;
		bool EqualCommonOption(const JBvhOption& tar)const noexcept;
	public:
		void Store(JFileIOTool& tool);
		void Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
	};
}
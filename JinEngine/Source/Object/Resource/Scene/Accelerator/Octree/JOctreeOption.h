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
#include"../JAcceleratorOption.h"

namespace JinEngine
{
	struct JOctreeOption
	{
	public:
		uint minSize = 32;
		uint octreeSizeSquare = 9;
		float looseFactor = 2;
	public:
		JAcceleratorOption commonOption;
	public:
		JOctreeOption() = default;
		JOctreeOption(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const JAcceleratorOption& commonOption);
	public:
		bool EqualOctreeOption(const JOctreeOption& tar)const noexcept;
		bool EqualCommonOption(const JOctreeOption& tar)const noexcept;
	public:
		void Store(JFileIOTool& tool);
		void Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
	};
}
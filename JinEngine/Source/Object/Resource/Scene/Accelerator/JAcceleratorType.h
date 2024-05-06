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
#include"../../../Component/RenderItem/JRenderLayer.h"
#include"../../../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	REGISTER_ENUM_CLASS(J_ACCELERATOR_TYPE, short, OCTREE = 0, BVH, KD_TREE)
	REGISTER_ENUM_CLASS(J_ACCELERATOR_SPLIT_TYPE, short, SAH = 0);
	REGISTER_ENUM_CLASS(J_ACCELERATOR_BUILD_TYPE, short, TOP_DOWN = 0);

	enum class J_ACCELERATOR_LAYER
	{
		COMMON_OBJECT,
		DEBUG_OBJECT,
		COUNT,
		INVALID,
	};

	static J_ACCELERATOR_LAYER ConvertAcceleratorLayer(const J_RENDER_LAYER layer)
	{
		switch (layer)
		{
		case JinEngine::J_RENDER_LAYER::OPAQUE_OBJECT:
			return J_ACCELERATOR_LAYER::COMMON_OBJECT;
		case JinEngine::J_RENDER_LAYER::SKY:
			return J_ACCELERATOR_LAYER::INVALID;
		case JinEngine::J_RENDER_LAYER::DEBUG_OBJECT:
			return J_ACCELERATOR_LAYER::DEBUG_OBJECT;
		case JinEngine::J_RENDER_LAYER::DEBUG_UI:
			return J_ACCELERATOR_LAYER::DEBUG_OBJECT;
		default:
			return J_ACCELERATOR_LAYER::INVALID;
		}
	}

	enum class J_ACCELERATOR_SORT_TYPE
	{
		NOT_USE,
		ASCENDING,
		DESCENDING
	};
}
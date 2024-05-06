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
#include"../../Core/Reflection/JReflection.h"
namespace JinEngine
{
	namespace Graphic
	{
		//HZB_OCC_OBJECT count is same as BOUNDING_OBJECT
		REGISTER_ENUM_CLASS(J_UPLOAD_FRAME_RESOURCE_TYPE, int, OBJECT, 
			SCENE_PASS,
			ANIMATION,
			CAMERA,
			BOUNDING_OBJECT,	//this frame count is same as OBJECT
			MATERIAL,
			DIRECTIONAL_LIGHT,
			CASCADE_SHADOW_MAP_INFO,
			POINT_LIGHT,
			SPOT_LIGHT,
			RECT_LIGHT,
			SHADOW_MAP_ARRAY_DRAW,
			SHADOW_MAP_CUBE_DRAW,
			SHADOW_MAP_DRAW,
			DEPTH_TEST_PASS,
			HZB_OCC_OBJECT,
			HZB_OCC_COMPUTE_PASS,
			LIGHT_CULLING_PASS,
			SSAO_PASS, 
			OBJECT_REF_INFO)

		enum class J_UPLOAD_CAPACITY_CONDITION
		{
			KEEP,
			DOWN_CAPACITY,
			UP_CAPACITY
		};
	}
}
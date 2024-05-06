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
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants
		{
			JVector4<float> albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			// Used in texture mapping.
			JMatrix4x4 matTransform = JMatrix4x4::Identity();
			float metallic = 0.75f;
			float roughness = 0.3f;
			float specularFactor = 0.5f;
			/*
			* srv heap index
			1 = missing
			*/
			uint albedoMapIndex = 1;
			uint normalMapIndex = 1;
			uint heightMapIndex = 1;
			uint metallicMapIndex = 1;
			uint roughnessMapIndex = 1;
			uint ambientOcclusionMapIndex = 1; 
			uint specularMapIndex = 1;
			uint materialPad00 = 0;
			uint materialPad01 = 0;
		};
	}
}
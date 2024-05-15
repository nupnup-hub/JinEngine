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
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JScenePassConstants
		{ 
			float appTotalTime = 0;
			float appDeltaTime = 0;
			float sceneTotalTime = 0;
			float sceneDeltaTime = 0;

			uint directionalLitSt = 0;
			uint directionalLitEd = 0;
			uint pointLitSt = 0;
			uint pointLitEd = 0;

			uint spotLitSt = 0;
			uint spotLitEd = 0;
			uint rectLitSt = 0;
			uint rectLitEd = 0;

			int missingTextureIndex = 0;		//always exist until engine end
			int bluseNoiseTextureIndex = 0;		//always exist until engine end
			JVector2F bluseNoiseTextureSize = JVector2F::Zero();

			JVector2F invBluseNoiseTextureSize = JVector2F::Zero();
			float clusterMinDepth = 0;
			int paddPad00 = 0; 
		};
	}
}
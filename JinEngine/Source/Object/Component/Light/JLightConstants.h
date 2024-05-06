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

namespace JinEngine
{
	namespace Constants
	{
		static constexpr int maxScenePerDircitonalLight = 4;

		static constexpr float globarLightMinDistance = 0.1f;
		static constexpr float globarLightMaxDistance = 2048;
		static constexpr float globarLightMaxDistanceHalf = globarLightMaxDistance * 0.5f;

		static constexpr float localLightMinDistance = 0.1f;
		static constexpr float localLightMaxDistance = 512;
		static constexpr float localLightMaxDistanceHalf = localLightMaxDistance * 0.5f;

		//for calculate perspective proj
		//static constexpr float lightNear = 1.0f;
		//static constexpr float lightNear = 1.0f;
		//static constexpr float lightMaxFar = lightMaxDistance;
		//static constexpr float lightMaxFarHalf = lightMaxFar * 0.5f;
		 
		static constexpr size_t minShadowMapSize = 256;
		static constexpr size_t maxShdowMapSize = 4096;

		static constexpr uint localLightCount = 3;	//point, spot, rect
	}
}
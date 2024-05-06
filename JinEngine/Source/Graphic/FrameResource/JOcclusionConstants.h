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
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Math/JMatrix.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JHzbOccObjectConstants
		{
			DirectX::XMFLOAT3 coners[8]
			{
				 DirectX::XMFLOAT3(0, 0, 0),DirectX::XMFLOAT3(0, 0, 0),
				 DirectX::XMFLOAT3(0, 0, 0),DirectX::XMFLOAT3(0, 0, 0),
				 DirectX::XMFLOAT3(0, 0, 0),DirectX::XMFLOAT3(0, 0, 0),
				 DirectX::XMFLOAT3(0, 0, 0),DirectX::XMFLOAT3(0, 0, 0)
			};
			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0, 0, 0);
			int queryResultIndex = 0;
			DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0, 0, 0);
			int isValid = 0;
		};
		struct JHzbOccDepthMapInfoConstants
		{
			int nowWidth = 0;
			int nowHeight = 0;
			int nowIndex = 0;
			int samplePad00 = 0;
		};		
		struct JHZBDebugInfo
		{
			JVector3F bboxV[8]
			{ 
				JVector3F::Zero(), JVector3F::Zero(), JVector3F::Zero(), JVector3F::Zero(),
				JVector3F::Zero(), JVector3F::Zero(), JVector3F::Zero(), JVector3F::Zero()
			};
			JVector3F nearPoint[6]
			{
				JVector3F::Zero(), JVector3F::Zero(), JVector3F::Zero(),
				JVector3F::Zero(), JVector3F::Zero(), JVector3F::Zero()
			};
			JVector3F center = JVector3F::Zero();
			JVector3F extents = JVector3F::Zero();
			JVector2F ndc[8]
			{
				JVector2F::Zero(), JVector2F::Zero(), JVector2F::Zero(), JVector2F::Zero(),
				JVector2F::Zero(), JVector2F::Zero(), JVector2F::Zero(), JVector2F::Zero()
			};
			float compareDepth[8];
			JVector4F minNearPoint = JVector4F::Zero();
			JVector4F clipNearH = JVector4F::Zero();
			JVector3F clipNearC = JVector3F::Zero();

			uint cullingRes = 0;
			float centerDepth = 0;
			float finalCompareDepth = 0;

			int threadIndex = 0;
			int queryIndex = 0;
			int debugPad00 = 0;
			int debugPad01 = 0;
			int debugPad02 = 0;
			int debugPad03 = 0;
		};
		struct JDepthTestConstants
		{
			JMatrix4x4 viewProj = JMatrix4x4::Identity();
		};
		struct JHzbOccComputeConstants
		{
			JMatrix4x4 view = JMatrix4x4::Identity();
			JMatrix4x4 proj = JMatrix4x4::Identity();
			DirectX::XMFLOAT4 frustumPlane[6];
			DirectX::XMFLOAT4 frustumDir;
			DirectX::XMFLOAT3 frustumPos;
			float viewWidth = 0;
			float viewHeight = 0;
			float camNear = 0;
			float camFar = 0;
			int validQueryCount = 0;
			int validQueryOffset = 0;
			int occMapCount = 0;
			int occIndexOffset = 0;
			int correctFailTrigger = 0;
			int usePerspective = 1;
			int hzbOccComputePad00 = 0;
			int hzbOccComputePad01 = 0;
			int hzbOccComputePad02 = 0;
		};
	}
}
#pragma once
#include"../../../Core/JDataType.h"
#include"../../../Utility/JMathHelper.h"
#include<DirectXMath.h>

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
			DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0, 0, 0);
			int queryResultIndex = 0;
			int isValid = 0; 
		};
		struct JHzbOccDepthMapInfoConstants
		{
			int nowWidth = 0;
			int nowHeight = 0;
			int nowIndex = 0;
			int samplePad00 = 0;
		};
		struct JHzbOccPassConstants
		{ 
			DirectX::XMFLOAT4X4 view = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 proj = JMathHelper::Identity4x4(); 
			DirectX::XMFLOAT4X4 viewProj = JMathHelper::Identity4x4();
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
		};

		struct HZBDebugInfo
		{
			DirectX::XMFLOAT4 plane[6]
			{
				 DirectX::XMFLOAT4(0, 0, 0, 0),DirectX::XMFLOAT4(0, 0, 0, 0),
				 DirectX::XMFLOAT4(0, 0, 0, 0),DirectX::XMFLOAT4(0, 0, 0, 0),
				 DirectX::XMFLOAT4(0, 0, 0, 0),DirectX::XMFLOAT4(0, 0, 0, 0)
			};
			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0, 0, 0);

			int cullingRes = 0;
			float centerDepth = 0;
			float finalCompareDepth = 0; 

			int threadIndex = 0;
			int queryIndex = 0;		
			int debugPad00 = 0;
		};

	}
}
#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicOption
		{
		public:
			bool isOcclusionQueryActivated = true; 
			bool isHDOcclusionAcitvated = false;
			bool isHZBOcclusionActivated = false; 
			bool allowHZBCorrectFail = true;
			bool allowDebugOutline = false;  
		public:
			bool IsHDOccActivated()const noexcept;
			bool IsHZBOccActivated()const noexcept;
		};
	}
}
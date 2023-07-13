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
		public:
			//Debug
			bool allowHZBCorrectFail = true;
			bool allowDebugOutline = false;
			bool allowMultiThread = true;
		public:
			bool IsHDOccActivated()const noexcept;
			bool IsHZBOccActivated()const noexcept;
		};
	}
}
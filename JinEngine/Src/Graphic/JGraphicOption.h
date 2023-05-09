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
			bool allowHZBCorrectFail = false;
			bool allowHZBDepthMapDebug = false;
			bool allowDebugOutline = false; 
		public: 
			float occUpdateFrequency = 0.1f;
		public:
			bool IsHDOccActivated()const noexcept;
			bool IsHZBOccActivated()const noexcept;
		};
	}
}
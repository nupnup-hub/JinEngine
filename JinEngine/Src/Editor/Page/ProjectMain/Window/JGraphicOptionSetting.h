#pragma once
namespace JinEngine
{
	namespace Editor
	{
		class JGraphicOptionSetting
		{  
		private:
			bool isOpenGraphicOptionViewer = false;
		public:
			void GraphicOptionOnScreen();
		public:
			bool IsOpenViewer()const noexcept;
			bool* GetOpenPtr() noexcept;
		};
	}
}
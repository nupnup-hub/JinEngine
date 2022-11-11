#pragma once
#include"../../Utility/JVector.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorCanvas
		{ 
		private:
			JVector2<float> preMousePos = JVector2<float>(0, 0);
			JVector2<float> mouseOffset = JVector2<float>(0, 0);
			float preWheelPos = 0;
			float canvasSize = 0;
			float zoom = 0;
			float minZoom = -75;
			float maxZoom = 75;
		public:
			void Clear();
		public:
			void Update();
			void DrawCanvas();
		public:			
			JVector2<float> GetMouseOffset()const noexcept;
			float GetCanvasSize()const noexcept;
			float GetZoomRate()const noexcept;
			void SetCanvasSize(const float newCanvasSize)noexcept;
			void SetMinZoom(const int newMaxZoomRate)noexcept;
			void SetMaxZoom(const int newMinZoomRate)noexcept;
		};
	}
}
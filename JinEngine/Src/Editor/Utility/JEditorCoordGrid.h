#pragma once
#include"../../Utility/JVector.h"
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JGameObject; 
	namespace Editor
	{
		class JEditorGuiCoordGrid
		{ 
		private:
			JVector2<float> preMousePos = JVector2<float>(0, 0);
			JVector2<float> mouseOffset = JVector2<float>(0, 0);
			float preWheelPos = 0;
			float gridSize = 0;
			float zoom = 0;
			float minZoom = -75;
			float maxZoom = 75;
		public:
			void Clear();
		public:
			void Update();
			void Draw();
		public:			
			JVector2<float> GetMouseOffset()const noexcept;
			float GetGridSize()const noexcept;
			float GetZoomRate()const noexcept;
			void SetGridSize(const float newGridSize)noexcept;
			void SetMaxZoomRate(const float newMaxZoomRate)noexcept;
			void SetMinZoomRate(const float newMinZoomRate)noexcept;
		};

		class JEditorSceneCoordGrid
		{ 
		private: 
			Core::JUserPtr<JGameObject> coordGrid;
		public:
			void MakeCoordGrid(JGameObject* parent);
			void Clear();
		public: 
			void Update(const JVector2<float> xzMoveOffset);
		};
	}
}
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
		private:
			float preWheelPos = 0;
			float zoom = 0;
			float minZoom = -75;
			float maxZoom = 75;
		private:
			uint gridSize = 0;
			uint lineCount = 128;
		public:
			void Clear();
		public:
			void Update();
			void Draw();
		public:			
			JVector2<float> GetMouseOffset()const noexcept;
			uint GetGridSize()const noexcept;
			float GetZoomRate()const noexcept;
			void SetGridSize(const uint newGridSize)noexcept;
			void SetMaxZoomRate(const float newMaxZoomRate)noexcept;
			void SetMinZoomRate(const float newMinZoomRate)noexcept;
		};

		class JEditorSceneCoordGrid
		{ 
		private: 
			JUserPtr<JGameObject> coordGrid;
		private:
			int lineCount = 128;	//ex) lineCount = 2 => create row line 2 and column line 2
			int lineScale = 128;
			int lineStep = 1;
		public:
			void MakeCoordGrid(const JUserPtr<JGameObject>& parent);
			void Clear();
		public: 
			void Update(const JVector2<float> xzMoveOffset);
		public:
			int GetLineCount()const noexcept; 
			int GetLineStep()const noexcept;
			int GetMinLineCount()const noexcept; 
			int GetMinLineStep()const noexcept;
			int GetMaxLineCount()const noexcept; 
			int GetMaxLineStep()const noexcept;
		public:
			//create row line x and column line x.
			void SetLineCount(const int value)noexcept;
			void SetLineStep(const int value)noexcept;
		private:
			void SetLineScale()noexcept;
		public:
			bool HasCoordGrid()const noexcept;
		};
	}
}
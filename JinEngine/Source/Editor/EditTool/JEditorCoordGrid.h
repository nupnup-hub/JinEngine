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
#include"../../Core/Math/JVector.h"
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
			int lineCount = 0;	//ex) lineCount = 2 => create row line 2 and column line 2
			int lineScale = 0;
			int lineStep = 0;
		public:
			JEditorSceneCoordGrid();
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
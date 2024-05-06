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
#include"../Gui/JGuiType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Reflection/JReflection.h" 
#include"../../Core/Interface/JTreeInterface.h"  
#include"../../Core/Geometry/JBBox.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorMouseDragBox;
		class JEditorSelectableStructure
		{ 
		private:
			//update per begin
			std::vector<Core::JBBox2D> lastUpdated;
			bool useFrame = false;
			bool useMultiColor = false;
			bool applyRounded = false;
			bool isRecordUpdateData = false;
		private:
			//optional
			JVector4F deltaColor = JVector4F::Zero();
			float frameThickness = 1;
		public:
			void Begin(const bool useFrame, const bool useMultiColor, const bool applyRounded, const bool isRecordUpdateData = false);
			void End();
		public:
			bool DisplaySelectable(const std::string& label, 
				J_GUI_SELECTABLE_FLAG_ flags, 
				const bool isFocus, 
				const bool isActivated, 
				const bool isSelected,
				const JVector2F size,
				const JVector4F color,
				const JVector4F frameColor = JVector4F::Zero()); 
		public:
			J_GUI_SELECTABLE_FLAG_ GetBaseFlag()const noexcept;
			std::vector<Core::JBBox2D> GetLastUpdated()const noexcept;
		public:
			void SetDeltaColor(const JVector4F& col)noexcept;
			void SetFrameThickness(const float thickness)noexcept;
		}; 
	}
}
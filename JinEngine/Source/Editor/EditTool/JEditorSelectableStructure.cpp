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


#include"JEditorSelectableStructure.h"
#include"JEditorMouseDragBox.h"
#include"../Gui/JGui.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/Geometry/JBBox.h"

namespace JinEngine::Editor
{ 
	void JEditorSelectableStructure::Begin(const bool useFrame, const bool useMultiColor, const bool applyRounded, const bool isRecordUpdateData)
	{
		JEditorSelectableStructure::useFrame = useFrame;
		JEditorSelectableStructure::useMultiColor = useMultiColor;
		JEditorSelectableStructure::applyRounded = applyRounded;
		JEditorSelectableStructure::isRecordUpdateData = isRecordUpdateData;

		if (lastUpdated.size() > 0)
			lastUpdated.clear();
	}
	void JEditorSelectableStructure::End()
	{ 
	}
	bool JEditorSelectableStructure::DisplaySelectable(const std::string& label,
		J_GUI_SELECTABLE_FLAG_ flags,
		const bool isFocus,
		const bool isActivated,
		const bool isSelected,
		const JVector2F size,
		const JVector4F color,
		const JVector4F frameColor)
	{ 
		uint styleCount = 1;
		if (!applyRounded)
		{
			JGui::PushStyle(J_GUI_STYLE::FRAME_ROUNDING, 0);
			++styleCount;
		}
		JGui::PushStyle(J_GUI_STYLE::FRAME_BORDER_SIZE, 0);
		JGui::PushTreeNodeColorSet(isFocus, isActivated, isSelected);	
		if (useFrame)
			JGui::DrawRectFrame(JGui::GetCursorScreenPos(), size, frameColor, frameThickness, true);
		if (useMultiColor)
			JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(), size, color, deltaColor, true);
		else
			JGui::DrawRectFilledColor(JGui::GetCursorScreenPos(), size, color, true);
		 
		const JVector2F preCursor = JGui::GetCursorScreenPos();
		const bool res = JGui::Selectable(label, isSelected, J_GUI_SELECTABLE_FLAG_ALLOW_DOUBLE_CLICK, size);
		JGui::PopTreeNodeColorSet(isActivated, isSelected);
		JGui::PopStyle(styleCount);
		if (isRecordUpdateData)
			lastUpdated.emplace_back(preCursor, preCursor + size);
		return res;
	}
	J_GUI_SELECTABLE_FLAG_ JEditorSelectableStructure::GetBaseFlag()const noexcept
	{
		return J_GUI_SELECTABLE_FLAG_ALLOW_DOUBLE_CLICK;
	}
	std::vector<Core::JBBox2D> JEditorSelectableStructure::GetLastUpdated()const noexcept
	{
		return lastUpdated;
	}
	void JEditorSelectableStructure::SetDeltaColor(const JVector4F& col)noexcept
	{
		deltaColor = col;
	}
	void JEditorSelectableStructure::SetFrameThickness(const float thickness)noexcept
	{
		frameThickness = thickness;
	}
}
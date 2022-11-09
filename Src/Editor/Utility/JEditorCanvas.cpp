#include"JEditorCanvas.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		void JEditorCanvas::Clear()
		{ 
			mouseOffset = JVector2<float>(0, 0);
		}
		void JEditorCanvas::Update()
		{
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (ImGui::IsMouseDown(1) || (ImGui::IsMouseDown(0) && !ImGui::IsItemClicked()))
				{
					const float halfCanvas = canvasSize * 0.5f;
					mouseOffset = mouseOffset + (ImGui::GetMousePos() - preMousePos);
					mouseOffset.x = std::clamp(mouseOffset.x, -halfCanvas, halfCanvas);
					mouseOffset.y = std::clamp(mouseOffset.y, -halfCanvas, halfCanvas);
				}
				zoom += ImGui::GetIO().MouseWheel;
				zoom = std::clamp(zoom, minZoom, maxZoom);
			}
			preMousePos = ImGui::GetMousePos();
			preWheelPos = ImGui::GetIO().MouseWheel;
		}
		void JEditorCanvas::DrawCanvas()
		{
			ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
			ImVec2 canvasSz = ImGui::GetContentRegionAvail();
			if (canvasSz.x < 50.0f) canvasSz.x = 50.0f;
			if (canvasSz.y < 50.0f) canvasSz.y = 50.0f;
			ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSz.x, canvasP0.y + canvasSz.y);

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->PushClipRect(canvasP0, canvasP1, true);

			const float GRID_STEP = 64.0f * GetZoomRate();
			for (float x = fmodf(mouseOffset.x, GRID_STEP); x < canvasSz.x; x += GRID_STEP)
				drawList->AddLine(ImVec2(canvasP0.x + x, canvasP0.y), ImVec2(canvasP0.x + x, canvasP1.y), IM_COL32(200, 200, 200, 40), 2.0f);
			for (float y = fmodf(mouseOffset.y, GRID_STEP); y < canvasSz.y; y += GRID_STEP)
				drawList->AddLine(ImVec2(canvasP0.x, canvasP0.y + y), ImVec2(canvasP1.x, canvasP0.y + y), IM_COL32(200, 200, 200, 40), 2.0f);
		}
		JVector2<float> JEditorCanvas::GetMouseOffset()const noexcept
		{
			return mouseOffset;
		}
		float JEditorCanvas::GetCanvasSize()const noexcept
		{
			return canvasSize;
		}
		float JEditorCanvas::GetZoomRate()const noexcept
		{
			return (100 + zoom) / 100;
		}
		void JEditorCanvas::SetCanvasSize(const float newCanvasSize)noexcept
		{
			canvasSize = newCanvasSize;
		}
		void JEditorCanvas::SetMinZoom(const int newMaxZoom)noexcept
		{
			maxZoom = newMaxZoom;
		}
		void JEditorCanvas::SetMaxZoom(const int newMinZoom)noexcept
		{
			minZoom = newMinZoom;
		}
	}
}
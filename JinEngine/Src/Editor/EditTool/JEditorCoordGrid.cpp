#include"JEditorCoordGrid.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/GameObject/JGameObjectCreator.h" 
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Material/JMaterial.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			static constexpr int gridFactor = 2048;
			static constexpr int gridLineCount = 128;
			static constexpr int step = gridFactor / gridLineCount;
			static constexpr float zoomRateRange = 75;
		}

		void JEditorGuiCoordGrid::Clear()
		{ 
			mouseOffset = JVector2<float>(0, 0);
		}
		void JEditorGuiCoordGrid::Update()
		{
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy))
			{ 
				if (ImGui::IsMouseDown(2))
				{ 
					const float halfCanvas = gridSize * 0.5f;
					mouseOffset = mouseOffset + (ImGui::GetMousePos() - preMousePos);
					mouseOffset.x = std::clamp(mouseOffset.x, -halfCanvas, halfCanvas);
					mouseOffset.y = std::clamp(mouseOffset.y, -halfCanvas, halfCanvas);
				}
				zoom += ImGui::GetIO().MouseWheel;
				zoom = std::clamp(zoom, minZoom, maxZoom);
			}
			preMousePos = ImGui::GetMousePos();
			preWheelPos = ImGui::GetIO().MouseWheel;
			ImGui::Text((std::to_string(mouseOffset.x) + "_" + std::to_string(mouseOffset.y)).c_str());
		}
		void JEditorGuiCoordGrid::Draw()
		{
			ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
			ImVec2 canvasSz = ImGui::GetContentRegionAvail();
			if (canvasSz.x < 50.0f) canvasSz.x = 50.0f;
			if (canvasSz.y < 50.0f) canvasSz.y = 50.0f;
			ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSz.x, canvasP0.y + canvasSz.y);

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->PushClipRect(canvasP0, canvasP1, true);

			const float GRID_STEP = Constants::gridLineCount * GetZoomRate();
			for (float x = fmodf(mouseOffset.x, GRID_STEP); x < canvasSz.x; x += GRID_STEP)
				drawList->AddLine(ImVec2(canvasP0.x + x, canvasP0.y), ImVec2(canvasP0.x + x, canvasP1.y), IM_COL32(200, 200, 200, 40), 2.0f);
			for (float y = fmodf(mouseOffset.y, GRID_STEP); y < canvasSz.y; y += GRID_STEP)
				drawList->AddLine(ImVec2(canvasP0.x, canvasP0.y + y), ImVec2(canvasP1.x, canvasP0.y + y), IM_COL32(200, 200, 200, 40), 2.0f);
		}
		JVector2<float> JEditorGuiCoordGrid::GetMouseOffset()const noexcept
		{
			return mouseOffset;
		}
		uint JEditorGuiCoordGrid::GetGridSize()const noexcept
		{
			return gridSize;
		}
		float JEditorGuiCoordGrid::GetZoomRate()const noexcept
		{
			return (100 + zoom) / 100;
		}
		void JEditorGuiCoordGrid::SetGridSize(const uint newGridSize)noexcept
		{
			gridSize = newGridSize;
			const float halfCanvas = gridSize * 0.5f; 
			mouseOffset.x = std::clamp(mouseOffset.x, -halfCanvas, halfCanvas);
			mouseOffset.y = std::clamp(mouseOffset.y, -halfCanvas, halfCanvas);
		}
		void JEditorGuiCoordGrid::SetMaxZoomRate(const float newMaxZoom)noexcept
		{
			maxZoom = std::clamp(newMaxZoom, 0.0f, Constants::zoomRateRange);
		}
		void JEditorGuiCoordGrid::SetMinZoomRate(const float newMinZoom)noexcept
		{
			minZoom = std::clamp(-newMinZoom, -Constants::zoomRateRange, 0.0f);
		}
		 
		void JEditorSceneCoordGrid::MakeCoordGrid(const JUserPtr<JGameObject>& parent)
		{ 
			coordGrid = JICI::Create<JGameObject>(L"SceneCoordGridRoot", Core::MakeGuid(), OBJECT_FLAG_EDITOR_OBJECT, parent);
			float posFactor = (Constants::gridLineCount / 2) * -Constants::step;

			//Line is yUp
			for (int i = 0; i < Constants::gridLineCount; ++i)
			{
				JUserPtr<JGameObject> newLine = JGCI::CreateDebugLineShape(coordGrid,
					OBJECT_FLAG_EDITOR_OBJECT, 
					J_DEFAULT_SHAPE::DEFAULT_SHAPE_LINE, 
					J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK,
					false);

				JUserPtr<JTransform> transform = newLine->GetTransform();
				transform->SetRotation(DirectX::XMFLOAT3(90, 0, 0));
				transform->SetPosition(DirectX::XMFLOAT3(posFactor + (i * Constants::step), 0, 0));
				transform->SetScale(DirectX::XMFLOAT3(0, 2000, 0));
			}
			for (int i = 0; i < Constants::gridLineCount; ++i)
			{
				JUserPtr<JGameObject> newLine = JGCI::CreateDebugLineShape(coordGrid,
					OBJECT_FLAG_EDITOR_OBJECT, 
					J_DEFAULT_SHAPE::DEFAULT_SHAPE_LINE,
					J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK,
					false);
				JUserPtr<JTransform>transform = newLine->GetTransform();
				transform->SetRotation(DirectX::XMFLOAT3(0, 0, 90));
				transform->SetPosition(DirectX::XMFLOAT3(0, 0, posFactor + (i * Constants::step)));
				transform->SetScale(DirectX::XMFLOAT3(0, 2000, 0));
			}
		}
		void JEditorSceneCoordGrid::Clear()
		{ 
			if(coordGrid.IsValid())
				JObject::BeginDestroy(coordGrid.Get());
		}
		void JEditorSceneCoordGrid::Update(const JVector2<float> xzMoveOffset)
		{
			if (!coordGrid.IsValid())
				return;

			const int xNowMovedFactor = (int)xzMoveOffset.x / Constants::step;
			const int zNowMovedFactor = (int)xzMoveOffset.y / Constants::step;

			JUserPtr<JTransform> transform = coordGrid->GetTransform();
			const JVector3<float> nowPos = transform->GetPosition();
			
			const int xPreMovedFactor = (int)(nowPos.x / Constants::step);
			const int zPreMovedFactor = (int)(nowPos.z / Constants::step);

			if (xNowMovedFactor != xPreMovedFactor || zNowMovedFactor != zPreMovedFactor)
			{
				transform->SetPosition(DirectX::XMFLOAT3((float)(xNowMovedFactor * Constants::step), 0.0f,
					(float)(zNowMovedFactor * Constants::step)));
			}
		}
		bool JEditorSceneCoordGrid::HasCoordGrid()const noexcept
		{
			return coordGrid.IsValid();
		}
	}
}
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
			static constexpr float zoomRateRange = 75;

			static constexpr int minLineCount = 2;
			static constexpr int minLineStep = 1;
			static constexpr int minLineScale = 1;

			static constexpr int maxLineCount = 512;
			static constexpr int maxLineStep = 64;
			static constexpr int maxLineScale = maxLineCount * maxLineStep;
		}
		namespace
		{
			static int ModifyLineCount(int newLineCount, const int preLineCount) noexcept
			{
				newLineCount = std::clamp(newLineCount, Constants::minLineCount, Constants::maxLineCount);
				if (newLineCount == Constants::minLineCount || newLineCount == Constants::maxLineCount)
					return newLineCount;

				if (newLineCount < preLineCount)
					return preLineCount / 2;
				else
					return preLineCount * 2;
				return newLineCount;
			}
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

			const float GRID_STEP = lineCount * GetZoomRate();
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
			const float posFactor = (lineCount / 2) * -lineStep;

			//Line is yUp
			for (int i = 0; i < lineCount; ++i)
			{
				JUserPtr<JGameObject> newLine = JGCI::CreateDebugLineShape(coordGrid,
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::LINE,
					J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK,
					false);

				JUserPtr<JTransform> transform = newLine->GetTransform();
				transform->SetRotation(DirectX::XMFLOAT3(90, 0, 0));
				transform->SetPosition(DirectX::XMFLOAT3(posFactor + (i * lineStep), 0, 0));
				transform->SetScale(DirectX::XMFLOAT3(1, lineScale, 1));
			}
			for (int i = 0; i < lineCount; ++i)
			{
				JUserPtr<JGameObject> newLine = JGCI::CreateDebugLineShape(coordGrid,
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::LINE,
					J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK,
					false);
				JUserPtr<JTransform>transform = newLine->GetTransform();
				transform->SetRotation(DirectX::XMFLOAT3(0, 0, 90));
				transform->SetPosition(DirectX::XMFLOAT3(0, 0, posFactor + (i * lineStep)));
				transform->SetScale(DirectX::XMFLOAT3(1, lineScale, 1));
			}
		}
		void JEditorSceneCoordGrid::Clear()
		{
			if (coordGrid.IsValid())
				JObject::BeginDestroy(coordGrid.Release());
		}
		void JEditorSceneCoordGrid::Update(const JVector2<float> xzMoveOffset)
		{
			if (!coordGrid.IsValid())
				return;

			const int xNowMovedFactor = (int)xzMoveOffset.x / lineStep;
			const int zNowMovedFactor = (int)xzMoveOffset.y / lineStep;

			JUserPtr<JTransform> transform = coordGrid->GetTransform();
			const JVector3<float> nowPos = transform->GetPosition();

			const int xPreMovedFactor = (int)(nowPos.x / lineStep);
			const int zPreMovedFactor = (int)(nowPos.z / lineStep);

			if (xNowMovedFactor != xPreMovedFactor || zNowMovedFactor != zPreMovedFactor)
			{
				transform->SetPosition(DirectX::XMFLOAT3((float)(xNowMovedFactor * lineStep), 0.0f,
					(float)(zNowMovedFactor * lineStep)));
			}
		}
		int JEditorSceneCoordGrid::GetLineCount()const noexcept
		{
			return lineCount;
		}
		int JEditorSceneCoordGrid::GetLineStep()const noexcept
		{
			return lineStep;
		}
		int JEditorSceneCoordGrid::GetMinLineCount()const noexcept
		{
			return Constants::minLineCount;
		}
		int JEditorSceneCoordGrid::GetMinLineStep()const noexcept
		{
			return Constants::minLineStep;
		}
		int JEditorSceneCoordGrid::GetMaxLineCount()const noexcept
		{
			return Constants::maxLineCount;
		}
		int JEditorSceneCoordGrid::GetMaxLineStep()const noexcept
		{
			return Constants::maxLineStep;
		}
		void JEditorSceneCoordGrid::SetLineCount(const int value)noexcept
		{
			if (lineCount == value)
				return;

			lineCount = ModifyLineCount(value, lineCount);
			if (coordGrid != nullptr)
			{
				auto preParent = coordGrid->GetParent();
				auto prePosition = coordGrid->GetTransform()->GetPosition();
				Clear();
				SetLineScale();
				MakeCoordGrid(preParent);
				coordGrid->GetTransform()->SetPosition(prePosition);
			}
			else
				SetLineScale();
		}
		void JEditorSceneCoordGrid::SetLineStep(const int value)noexcept
		{
			if (lineStep == value)
				return;

			lineStep = std::clamp(value, GetMinLineStep(), GetMaxLineStep());
			if (coordGrid != nullptr)
			{
				auto children = coordGrid->GetChildren();
				const float posFactor = (lineCount / 2) * -lineStep;
				const uint rowOffset = children.size() / 2;

				for (int i = 0; i < lineCount; ++i)
					children[i]->GetTransform()->SetPosition(DirectX::XMFLOAT3(posFactor + (i * lineStep), 0, 0));
				for (int i = 0; i < lineCount; ++i)
					children[i + rowOffset]->GetTransform()->SetPosition(DirectX::XMFLOAT3(0, 0, posFactor + (i * lineStep)));
			}
			SetLineScale();
		}
		void JEditorSceneCoordGrid::SetLineScale()noexcept
		{
			int newLineScale = lineCount * lineStep;
			lineScale = std::clamp(newLineScale, Constants::minLineScale, Constants::maxLineScale);
			if (coordGrid != nullptr)
			{
				auto children = coordGrid->GetChildren();
				for (const auto& data : children)
					data->GetTransform()->SetScale(DirectX::XMFLOAT3(1, lineScale, 1));
			}
		}
		bool JEditorSceneCoordGrid::HasCoordGrid()const noexcept
		{
			return coordGrid.IsValid();
		}
	}
}
#include"JEditorCoordGrid.h"
#include"../Gui/JGui.h"
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/GameObject/JGameObjectCreator.h" 
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Material/JMaterial.h"
#include"../../Object/Resource/Mesh//JMeshGeometry.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static constexpr float zoomRateRange = 75;

			static constexpr int minLineCount = 2;
			static constexpr int minLineStep = 1;
			static constexpr int minLineScale = 1;

			static constexpr int maxLineCount = 256;
			static constexpr int maxLineStep = 64;
			static constexpr int maxLineScale = maxLineCount * maxLineStep;
		}
		namespace
		{
			static int ModifyLineCount(int newLineCount, const int preLineCount) noexcept
			{
				newLineCount = std::clamp(newLineCount, Private::minLineCount, Private::maxLineCount);
				return newLineCount % 2 ? newLineCount + 1 : newLineCount;
				/*
				if (newLineCount == Private::minLineCount || newLineCount == Private::maxLineCount)
					return newLineCount;

				if (newLineCount < preLineCount)
					return preLineCount / 2;
				else
					return preLineCount * 2;
				return newLineCount;
				*/
			}
		}

		void JEditorGuiCoordGrid::Clear()
		{
			mouseOffset = JVector2<float>(0, 0);
		}
		void JEditorGuiCoordGrid::Update()
		{
			if (JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW | J_GUI_FOCUS_FLAG_DOCK_HIERARCHY))
			{
				if (JGui::IsMouseDown(Core::J_MOUSE_BUTTON::MIDDLE))
				{
					const float halfCanvas = gridSize * 0.5f;
					mouseOffset = mouseOffset + (JGui::GetMousePos() - preMousePos);
					mouseOffset.x = std::clamp(mouseOffset.x, -halfCanvas, halfCanvas);
					mouseOffset.y = std::clamp(mouseOffset.y, -halfCanvas, halfCanvas);
				}
				zoom += JGui::GetMouseWheel();
				zoom = std::clamp(zoom, minZoom, maxZoom);
			}
			preMousePos = JGui::GetMousePos();
			preWheelPos = JGui::GetMouseWheel();
			JGui::Text((std::to_string(mouseOffset.x) + "_" + std::to_string(mouseOffset.y)));
		}
		void JEditorGuiCoordGrid::Draw()
		{
			JVector2<uint> canvasP0 = JGui::GetCursorScreenPos();
			JVector2<float> canvasSz = JGui::GetRestWindowContentsSize();
			if (canvasSz.x < 50.0f) canvasSz.x = 50.0f;
			if (canvasSz.y < 50.0f) canvasSz.y = 50.0f;
			JVector2<uint> canvasP1(canvasP0.x + canvasSz.x, canvasP0.y + canvasSz.y);
			 
			JGui::PushClipRect(canvasP0, canvasP1, true);
			const float GRID_STEP = lineCount * GetZoomRate();
			for (float x = fmodf(mouseOffset.x, GRID_STEP); x < canvasSz.x; x += GRID_STEP)
				JGui::AddLine(JVector2F(canvasP0.x + x, canvasP0.y), JVector2F(canvasP0.x + x, canvasP1.y), JVector4F(0.8f, 0.8f, 0.8f, 0.15f), 2.0f);
			for (float y = fmodf(mouseOffset.y, GRID_STEP); y < canvasSz.y; y += GRID_STEP)
				JGui::AddLine(JVector2F(canvasP0.x, canvasP0.y + y), JVector2F(canvasP1.x, canvasP0.y + y), JVector4F(0.8f, 0.8f, 0.8f, 0.15f), 2.0f);
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
			maxZoom = std::clamp(newMaxZoom, 0.0f, Private::zoomRateRange);
		}
		void JEditorGuiCoordGrid::SetMinZoomRate(const float newMinZoom)noexcept
		{
			minZoom = std::clamp(-newMinZoom, -Private::zoomRateRange, 0.0f);
		}

		JEditorSceneCoordGrid::JEditorSceneCoordGrid()
		{
			lineCount = 256;
			lineStep = 2;
			SetLineScale();
		}
		void JEditorSceneCoordGrid::MakeCoordGrid(const JUserPtr<JGameObject>& parent)
		{
			coordGrid = JICI::Create<JGameObject>(L"SceneCoordGridRoot", Core::MakeGuid(), OBJECT_FLAG_EDITOR_OBJECT, parent);
			float posFactor = (lineCount / 2) * -lineStep;
			if ((lineCount % 2) == 0)
				posFactor += lineStep * 0.5f;

			//Line is yUp
			for (int i = 0; i < lineCount; ++i)
			{
				JUserPtr<JGameObject> newLine = JGCI::CreateDebugLineShape(coordGrid,
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::LINE,
					J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK,
					false);

				JUserPtr<JTransform> transform = newLine->GetTransform();
				transform->SetRotation(JVector3<float>(90, 0, 0));
				transform->SetPosition(JVector3<float>(posFactor + (i * lineStep), 0, 0));
				transform->SetScale(JVector3<float>(1, lineScale, 1));
			}
			for (int i = 0; i < lineCount; ++i)
			{
				JUserPtr<JGameObject> newLine = JGCI::CreateDebugLineShape(coordGrid,
					OBJECT_FLAG_EDITOR_OBJECT,
					J_DEFAULT_SHAPE::LINE,
					J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK,
					false);
				JUserPtr<JTransform>transform = newLine->GetTransform();
				transform->SetRotation(JVector3<float>(0, 0, 90));
				transform->SetPosition(JVector3<float>(0, 0, posFactor + (i * lineStep)));
				transform->SetScale(JVector3<float>(1, lineScale, 1));
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
				transform->SetPosition(JVector3<float>((float)(xNowMovedFactor * lineStep), 0.0f,
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
			return Private::minLineCount;
		}
		int JEditorSceneCoordGrid::GetMinLineStep()const noexcept
		{
			return Private::minLineStep;
		}
		int JEditorSceneCoordGrid::GetMaxLineCount()const noexcept
		{
			return Private::maxLineCount;
		}
		int JEditorSceneCoordGrid::GetMaxLineStep()const noexcept
		{
			return Private::maxLineStep;
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
					children[i]->GetTransform()->SetPosition(JVector3<float>(posFactor + (i * lineStep), 0, 0));
				for (int i = 0; i < lineCount; ++i)
					children[i + rowOffset]->GetTransform()->SetPosition(JVector3<float>(0, 0, posFactor + (i * lineStep)));
			}
			SetLineScale();
		}
		void JEditorSceneCoordGrid::SetLineScale()noexcept
		{
			auto line = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LINE);
			float length = line->GetBoundingBoxExtent().y * 2;

			int newLineScale = ((lineCount - 1) * lineStep) / length;
			//int newLineScale = lineCount * lineStep;
			lineScale = newLineScale; //std::clamp(newLineScale, Private::minLineScale, Private::maxLineScale);
			if (coordGrid != nullptr)
			{
				auto children = coordGrid->GetChildren();
				for (const auto& data : children)
					data->GetTransform()->SetScale(JVector3<float>(1, lineScale, 1));
			}
		}
		bool JEditorSceneCoordGrid::HasCoordGrid()const noexcept
		{
			return coordGrid.IsValid();
		}
	}
}
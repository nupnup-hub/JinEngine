#include"JEditorMouseDragBox.h"
#include"JEditorSceneImageInteraction.h"
#include"../Gui/JGui.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Core/Time/JStopWatch.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Utility/JCommonUtility.h"

//Debug
#include"../../Develop/Debug/JDevelopDebug.h"
namespace JinEngine::Editor
{
	namespace Private
	{
		static constexpr float canvasFactor = 0.985f;
		static JVector4<float> RectColor()noexcept
		{
			return JVector4<float>(0.15f, 0.15f, 0.5f, 0.5f);
		}
		static JVector4<float> RectDeltaColor()noexcept
		{
			return JVector4<float>(0.05f, 0.05f, 0.05f, 0.2f);
		}
		static JVector4<float> RectFrameColor()noexcept
		{
			return JVector4<float>(0.15f, 0.15f, 0.9f, 1.0f);
		}
		static void CanvasMinMax(_Out_ JVector2<int>& minV2,
			_Out_ JVector2<int>& maxV2,
			const JVector2<int>& wndPos = JGui::GetWindowPos(),
			const JVector2<int>& wndSize = JGui::GetWindowSize())
		{
			minV2 = wndPos + wndSize * (1 - canvasFactor);
			maxV2 = wndPos + wndSize * canvasFactor;
		}
	}

	JEditorMouseDragBox::UpdateIn::UpdateIn(const bool allowActivateBBox,
		JVector2<int> minOffset,
		JVector2<int> maxOffset)
		:allowActivateBBox(allowActivateBBox), minOffset(minOffset), maxOffset(maxOffset)
	{

	}

	void JEditorMouseDragBox::Update(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept
	{ 
		if (!IsActivated())
		{
			out.successUpdate = false;
			if (in.allowActivateBBox)
			{
				Activate();
				out.successActivateCanUpdateNextFrame = true;
				return;
			}
			else
				return;
		}

		if (CanDeActivate())
		{
			DeActivate();
			out.successUpdate = false;
			return;
		}

		//has order dependency! 
		const JVector2<uint> newMousePos = JGui::GetMousePos();
		if (newMousePos == uData.edMousePos)
			out.isSameMousePos = true;
		uData.edMousePos = newMousePos;

		JVector2<int> minV2;
		JVector2<int> maxV2;
		GetBBoxMinMax(minV2, maxV2, in.minOffset, in.maxOffset);

		JVector2<int> canvasMin;
		JVector2<int> canvasMax;
		Private::CanvasMinMax(canvasMin, canvasMax, uData.stWndPos, uData.stWndSize);
  
		JVector2<int> framePadding = JVector2<int>(1, 1);
		JGui::DrawRectFilledMultiColor(minV2 + framePadding, maxV2 - minV2 - framePadding, Private::RectColor(), Private::RectDeltaColor(), true);
		JGui::DrawRectFrame(minV2, maxV2 - minV2, Private::RectFrameColor(), framePadding.x,  true);
		
		out.successUpdate = true;
	}
	void JEditorMouseDragBox::GetBBoxMinMax(_Out_ JVector2<int>& minV2, 
		_Out_ JVector2<int>& maxV2,
		const JVector2<int>& minOffset,
		const JVector2<int>& maxOffset)const noexcept
	{
		if (!IsActivated())
		{
			minV2 = JVector2<int>::Zero();
			maxV2 = JVector2<int>::Zero();
			return;
		}

		JVector2<int> canvasMin;
		JVector2<int> canvasMax;
		Private::CanvasMinMax(canvasMin, canvasMax, uData.stWndPos, uData.stWndSize);

		minV2 = JVector2<int>::Min(uData.stMousePos, uData.edMousePos);
		maxV2 = JVector2<int>::Max(uData.stMousePos, uData.edMousePos);

		minV2.x = std::clamp(minV2.x, canvasMin.x + minOffset.x, minV2.x);
		minV2.y = std::clamp(minV2.y, canvasMin.y + minOffset.y, minV2.y);
		maxV2.x = std::clamp(maxV2.x, maxV2.x, canvasMax.x - maxOffset.x);
		maxV2.y = std::clamp(maxV2.y, maxV2.y, canvasMax.y - maxOffset.y);
	}
	bool JEditorMouseDragBox::CanActivate()const noexcept
	{
		if (!JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT))
			return false;

		JVector2<int> canvasMin;
		JVector2<int> canvasMax;
		Private::CanvasMinMax(canvasMin, canvasMax);

		return JGui::IsMouseInRect(canvasMin, canvasMax - canvasMin);
	}
	bool JEditorMouseDragBox::CanDeActivate()const noexcept
	{
		return !JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT) || 
			uData.stWndPos != JGui::GetWindowPos() ||
			uData.stWndSize != JGui::GetWindowSize();
	}
	void JEditorMouseDragBox::DoActivate() noexcept
	{
		Core::JActivatedInterface::DoActivate();
		uData.stMousePos = JGui::GetMousePos();
		uData.stWndPos = JGui::GetWindowPos();
		uData.stWndSize = JGui::GetWindowSize();
	}
	void JEditorMouseDragBox::DoDeActivate() noexcept
	{
		uData.stMousePos = JVector2<uint>::Zero();
		uData.edMousePos = JVector2<uint>::Zero();
		uData.stWndPos = JVector2<uint>::Zero();
		uData.stWndSize = JVector2<uint>::Zero();
		Core::JActivatedInterface::DoDeActivate();
	}

	JEditorMouseIdenDragBox::UpdateIn::UpdateIn(const bool allowActivateBBox,
		JVector2<float> sceneImageMinPoint,
		JUserPtr<JScene> scene,
		JUserPtr<JCamera> cam,
		J_ACCELERATOR_LAYER layer)
		:JEditorMouseDragBox::UpdateIn(allowActivateBBox, sceneImageMinPoint),
		scene(scene), cam(cam), layer(layer)
	{

	}

	JEditorMouseIdenDragBox::SelectedInfo::SelectedInfo(const JUserPtr<Core::JIdentifier>& iden, const bool isSelectedBefore)
		:iden(iden),isSelectedBefore(isSelectedBefore), isLastUpdateSelected(false)
	{}

	void JEditorMouseIdenDragBox::UpdateSceneImageDrag(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept
	{  
		JEditorMouseDragBox::Update(in, out);
		if (!out.successUpdate || out.isSameMousePos)
			return;

		JVector2<int> minV2;
		JVector2<int> maxV2;
		GetBBoxMinMax(minV2, maxV2, in.minOffset);
 
		std::vector<JUserPtr<JGameObject>> selectedVec = JEditorSceneImageInteraction::Contain(in.scene,
			in.cam,
			J_ACCELERATOR_LAYER::COMMON_OBJECT,
			in.minOffset,
			minV2,
			maxV2);

		bool(*equalPtr)(const SelectedInfo&, const size_t) = [](const SelectedInfo& info, const size_t guid) {return info.iden.IsValid() ? info.iden->GetGuid() == guid : false; };
		if (selectedVec.size() > 0)
		{
			if (isHoldCtrl && !JGui::IsKeyPressed(Core::J_KEYCODE::CONTROL))
			{
				isHoldCtrl = false;
				for (int i = 0; i < sInfoVec.size(); ++i)
				{
					if (sInfoVec[i].isSelectedBefore)
					{
						sInfoVec.erase(sInfoVec.begin() + i);
						--i;
					}
				}
			}

			for (const auto& data : selectedVec)
			{
				int index = JCUtil::GetIndex(sInfoVec, equalPtr, data->GetGuid());
				if (index == invalidIndex)
				{
					sInfoVec.push_back(SelectedInfo(data, data->IsSelected()));
					sInfoVec[sInfoVec.size() - 1].isLastUpdateSelected = true;

					if (data->IsSelected())
						out.newDeSelectedVec.push_back(data);
					else
						out.newSelectedVec.push_back(data);
				}
				else
					sInfoVec[index].isLastUpdateSelected = true;
			}  
		}  
		for (int i = 0; i < sInfoVec.size(); ++i)
		{
			if (!sInfoVec[i].isLastUpdateSelected)
			{
				if (sInfoVec[i].isSelectedBefore)
					out.newSelectedVec.push_back(sInfoVec[i].iden);
				else
					out.newDeSelectedVec.push_back(sInfoVec[i].iden);
				sInfoVec.erase(sInfoVec.begin() + i);
				--i;
			}
			else
				sInfoVec[i].isLastUpdateSelected = false;
		}
	}
	uint JEditorMouseIdenDragBox::GetSelectedCount()const noexcept
	{
		return sInfoVec.size();
	}
	void JEditorMouseIdenDragBox::DoActivate() noexcept
	{
		JEditorMouseDragBox::DoActivate();
		isHoldCtrl = JGui::IsKeyDown(Core::J_KEYCODE::CONTROL);
	}
	void JEditorMouseIdenDragBox::DoDeActivate() noexcept
	{
		sInfoVec.clear();
		JEditorMouseDragBox::DoDeActivate();
	}
}
#include"JEditorMouseDragBox.h"
#include"JEditorSceneImageInteraction.h"
#include"../Gui/JGui.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Core/Time/JStopWatch.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Utility/JCommonUtility.h"

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
			//minV2 = wndPos + wndSize * (1 - canvasFactor);
			minV2 = wndPos;
			maxV2 = wndPos + wndSize * canvasFactor;
		}
	}

	JEditorMouseDragBox::UpdateIn::UpdateIn(const bool allowActivateDragBox,
		JVector2<int> minOffset,
		JVector2<int> maxOffset)
		:allowActivateDragBox(allowActivateDragBox), minOffset(minOffset), maxOffset(maxOffset)
	{

	}
	void JEditorMouseDragBox::Update(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept
	{
		lastUpdateInCash = &in; 
		if (!IsActivated())
		{
			out.successUpdate = false;
			if (in.allowActivateDragBox)
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
		JGui::DrawRectFrame(minV2, maxV2 - minV2, Private::RectFrameColor(), framePadding.x, true);

		out.successUpdate = true;
		lastUpdateInCash = nullptr;
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
		Private::CanvasMinMax(canvasMin, canvasMax, JGui::GetWindowPos() + lastUpdateInCash->minOffset, JGui::GetWindowSize() - lastUpdateInCash->minOffset);

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

	JEditorIdentifierDragInterface::SelectedInfo::SelectedInfo(const JUserPtr<Core::JIdentifier>& iden, const bool isSelectedBefore)
		:iden(iden), isSelectedBefore(isSelectedBefore), isLastUpdateSelected(false)
	{}
	void JEditorIdentifierDragInterface::Update(const IdenVec& vec, const UpdateIn& in, UpdateOut& out)
	{
		if (vec.size() > 0)
		{
			TryClearHoldState();
			StuffSelectedBuffer(vec, in, out);
		}
		StuffOutBuffer(out);
	} 
	void JEditorIdentifierDragInterface::TryClearHoldState()
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
	}
	void JEditorIdentifierDragInterface::StuffSelectedBuffer(const IdenVec& vec, const UpdateIn& in, UpdateOut& out)
	{
		bool(*equalPtr)(const SelectedInfo&, const size_t) = [](const SelectedInfo& info, const size_t guid) {return info.iden.IsValid() ? info.iden->GetGuid() == guid : false; };
		for (const auto& data : vec)
		{
			int index = JCUtil::GetIndex(sInfoVec, equalPtr, data->GetGuid());
			if (index == invalidIndex)
			{
				const bool isSelecetd = IsSelected(data, in);
				sInfoVec.push_back(SelectedInfo(data, isSelecetd));
				sInfoVec[sInfoVec.size() - 1].isLastUpdateSelected = true;

				if (isSelecetd)
					out.newDeSelectedVec.push_back(data);
				else
					out.newSelectedVec.push_back(data);
			}
			else
				sInfoVec[index].isLastUpdateSelected = true;
		}
	}
	void JEditorIdentifierDragInterface::StuffOutBuffer(UpdateOut& out)
	{
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
	uint JEditorIdentifierDragInterface::GetSelectedCount()const noexcept
	{
		return sInfoVec.size();
	}
	void JEditorIdentifierDragInterface::DoActivate() noexcept
	{
		JEditorMouseDragBox::DoActivate();
		isHoldCtrl = JGui::IsKeyDown(Core::J_KEYCODE::CONTROL);
	}
	void JEditorIdentifierDragInterface::DoDeActivate() noexcept
	{
		sInfoVec.clear();
		JEditorMouseDragBox::DoDeActivate();
	}

	JEditorMouseDragSceneBox::UpdateIn::UpdateIn(const bool allowActivateDragBox,
		JVector2<float> minOffset,
		JVector2<float> maxOffset,
		JVector2<float> sceneImageScreenMinPoint,
		JUserPtr<JScene> scene,
		JUserPtr<JCamera> cam,
		J_ACCELERATOR_LAYER layer)
		:JEditorIdentifierDragInterface::UpdateIn(allowActivateDragBox, minOffset, maxOffset),
		scene(scene), cam(cam), layer(layer), sceneImageScreenMinPoint(sceneImageScreenMinPoint)
	{

	}
	void JEditorMouseDragSceneBox::UpdateSceneImageDrag(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept
	{
		JEditorMouseDragBox::Update(in, out);
		if (!out.successUpdate || out.isSameMousePos)
			return;

		JVector2<int> minV2;
		JVector2<int> maxV2;
		GetBBoxMinMax(minV2, maxV2, in.minOffset, in.maxOffset);

		std::vector<JUserPtr<JGameObject>> selectedVec = JEditorSceneImageInteraction::Contain(in.scene,
			in.cam,
			J_ACCELERATOR_LAYER::COMMON_OBJECT,
			in.sceneImageScreenMinPoint,
			minV2,
			maxV2);
 
		std::vector<JUserPtr<Core::JIdentifier>> selectedIdenVec;
		selectedIdenVec.insert(selectedIdenVec.begin(), std::move_iterator(selectedVec.begin()), std::move_iterator(selectedVec.end())); 
		Update(selectedIdenVec, in, out);
	} 
	bool JEditorMouseDragSceneBox::IsSelected(const JUserPtr<Core::JIdentifier>& iden, const JEditorIdentifierDragInterface::UpdateIn& in)const noexcept
	{
		return static_cast<JGameObject*>(iden.Get())->IsSelected();
	}
	void JEditorMouseDragSceneBox::DoActivate() noexcept
	{
		JEditorIdentifierDragInterface::DoActivate();
	}
	void JEditorMouseDragSceneBox::DoDeActivate() noexcept
	{ 
		JEditorIdentifierDragInterface::DoDeActivate();
	}

	JEditorMouseDragCanvas::IdenData::IdenData(const JUserPtr<Core::JIdentifier>& iden, const Core::JBBox2D& bbox)
		:iden(iden), bbox(bbox)
	{}
	JEditorMouseDragCanvas::UpdateIn::UpdateIn(const bool allowActivateDragBox,
		JVector2<float> minOffset,
		JVector2<float> maxOffset)
		:JEditorIdentifierDragInterface::UpdateIn(allowActivateDragBox, minOffset, maxOffset)
	{}
	JEditorMouseDragCanvas::UpdateIn::UpdateIn(const bool allowActivateDragBox,
		JVector2<float> minOffset,
		JVector2<float> maxOffset,
		std::vector<IdenData>&& iden,
		std::unordered_map<size_t, JUserPtr<Core::JIdentifier>>&& selectedMap)
		:JEditorIdentifierDragInterface::UpdateIn(allowActivateDragBox, minOffset, maxOffset),
		iden(std::move(iden)),
		selectedMap(std::move(selectedMap))
	{}
	void JEditorMouseDragCanvas::UpdateCanvasDrag(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept
	{
		JEditorMouseDragBox::Update(in, out);
		if (!out.successUpdate || out.isSameMousePos)
			return;

		JVector2<int> minV2;
		JVector2<int> maxV2;
		GetBBoxMinMax(minV2, maxV2, in.minOffset, in.maxOffset);
 
		const Core::JBBox2D tar(minV2, maxV2);
		std::vector<JUserPtr<Core::JIdentifier>> intersected;
		for (const auto& data : in.iden)
		{
			if (tar.Intersect(data.bbox))
				intersected.push_back(data.iden);
		}

		Update(intersected, in, out);
	} 
	std::vector<JEditorMouseDragCanvas::IdenData> JEditorMouseDragCanvas::Combine(const std::vector<JUserPtr<Core::JIdentifier>>& idenVec, const std::vector<Core::JBBox2D>& bboxVec)
	{
		std::vector<JEditorMouseDragCanvas::IdenData> result;
		const uint idenCount = (uint)idenVec.size();
		if (idenCount != bboxVec.size())
			return result;

		result.resize(idenCount);
		for (uint i = 0; i < idenCount; ++i)
			result[i] = IdenData(idenVec[i], bboxVec[i]);
		return result;
	}
	bool JEditorMouseDragCanvas::IsSelected(const JUserPtr<Core::JIdentifier>& iden, const JEditorIdentifierDragInterface::UpdateIn& in)const noexcept
	{
		auto thisIn = static_cast<const UpdateIn*>(&in); 
		return thisIn->selectedMap.find(iden->GetGuid()) != thisIn->selectedMap.end();
	}
	void JEditorMouseDragCanvas::DoActivate() noexcept
	{
		JEditorIdentifierDragInterface::DoActivate();
	}
	void JEditorMouseDragCanvas::DoDeActivate() noexcept
	{
		JEditorIdentifierDragInterface::DoDeActivate();
	}
}
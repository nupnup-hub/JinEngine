#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/Interface/JActivatedInterface.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Math/JVector.h"
#include"../../Object/Resource/Scene/Accelerator/JAcceleratorType.h"
#include<vector>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
	}

	class JScene;
	class JCamera;

	namespace Editor
	{
		class JEditorMouseDragBox : public Core::JActivatedInterface
		{
		private:
			struct UpdateData
			{
			public:
				JVector2<uint> stMousePos;
				JVector2<uint> edMousePos;
			public:
				JVector2<uint> stWndPos;
				JVector2<uint> stWndSize;
			};
		public:
			struct UpdateIn
			{
			public:
				// minOffset is added bbox min point
				JVector2<int> minOffset = JVector2<int>::Zero();
				//maxOffset is subtracted bbox max point
				JVector2<int> maxOffset = JVector2<int>::Zero();
			public:
				const bool allowActivateBBox = true;
			public:
				UpdateIn() = default;
				UpdateIn(const bool allowActivateBBox, 
					JVector2<int> minOffset = JVector2<int>::Zero(),
					JVector2<int> maxOffset = JVector2<int>::Zero());
			};
			struct UpdateOut
			{
			public:
				JVector2<int> minV2;
				JVector2<int> maxV2;
			public:
				bool isSameMousePos = false;
				bool successUpdate = false;
				bool successActivateCanUpdateNextFrame = false;
			};
		private:
			UpdateData uData;
		public:
			/*
			* @brief automacally manage class activation and can update on next frame after activate
			*/
			void Update(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept;
		protected:
			/*
			* @param minOffset is added minV2
			* @param maxOffset is subtracted maxV2
			*/
			void GetBBoxMinMax(_Out_ JVector2<int>& minV2,
				_Out_ JVector2<int>& maxV2,
				const JVector2<int>& minOffset = JVector2<int>::Zero(),
				const JVector2<int>& maxOffset = JVector2<int>::Zero())const noexcept;
		private:
			bool CanActivate()const noexcept final;
			bool CanDeActivate()const noexcept final;
		protected:
			void DoActivate() noexcept override;
			void DoDeActivate() noexcept override;
		};

		class JEditorMouseIdenDragBox final : public JEditorMouseDragBox
		{
		public:
			struct UpdateIn : public JEditorMouseDragBox::UpdateIn
			{
			public:
				JUserPtr<JScene> scene;
				JUserPtr<JCamera> cam;
				J_ACCELERATOR_LAYER layer; 
			public:
				JVector2F sceneImageScreenMinPoint;
			public:
				UpdateIn(const bool allowActivateBBox,
					JVector2<float> minOffset,
					JVector2<float> maxOffset,
					JVector2<float> sceneImageScreenMinPoint,
					JUserPtr<JScene> scene,
					JUserPtr<JCamera> cam,
					J_ACCELERATOR_LAYER layer);
			};
			struct UpdateOut : public JEditorMouseDragBox::UpdateOut
			{
			public:
				std::vector<JUserPtr<Core::JIdentifier>> newSelectedVec;
				std::vector<JUserPtr<Core::JIdentifier>> newDeSelectedVec;
			};
		private:
			struct SelectedInfo
			{
			public:
				JUserPtr<Core::JIdentifier> iden;
			public:
				bool isSelectedBefore = false;
				bool isLastUpdateSelected = false;
			public:
				SelectedInfo(const JUserPtr<Core::JIdentifier>& iden, const bool isSelectedBefore);
			};
		private:
			std::vector<SelectedInfo> sInfoVec;
			bool isHoldCtrl = false;	//test code
		public:
			void UpdateSceneImageDrag(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept;
		public:
			uint GetSelectedCount()const noexcept;
		private:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
		};
	}
}
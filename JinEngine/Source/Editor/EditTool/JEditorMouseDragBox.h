#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/Interface/JActivatedInterface.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Geometry/JBBox.h"
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
	class JGameObject; 

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
				// minOffset is added bbox min point	local cursor(o), world cursor(x)
				JVector2<int> minOffset = JVector2<int>::Zero();
				//maxOffset is subtracted bbox max point	local cursor(o), world cursor(x)
				JVector2<int> maxOffset = JVector2<int>::Zero();
			public:
				const bool allowActivateDragBox = true;
			public: 
				UpdateIn(const bool allowActivateDragBox, 
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
			const UpdateIn* lastUpdateInCash = nullptr;
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
		class JEditorIdentifierDragInterface : public JEditorMouseDragBox
		{
		protected:
			using GObjVec = std::vector<JUserPtr<JGameObject>>;
			using IdenVec = std::vector<JUserPtr<Core::JIdentifier>>;
			using IdenMap = std::unordered_map<size_t, JUserPtr<Core::JIdentifier>>;
		protected:
			struct UpdateOut : public JEditorMouseDragBox::UpdateOut
			{
			public:
				std::vector<JUserPtr<Core::JIdentifier>> newSelectedVec;
				std::vector<JUserPtr<Core::JIdentifier>> newDeSelectedVec;
			};
		protected:
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
			bool isHoldCtrl = false;	//test Code
		protected: 
			void Update(const IdenVec& vec, const UpdateIn& in, UpdateOut& out);
		private:
			void TryClearHoldState();
			void StuffSelectedBuffer(const IdenVec& vec, const UpdateIn& in, UpdateOut& out);
			void StuffOutBuffer(UpdateOut& out);
		public:
			uint GetSelectedCount()const noexcept;
		protected:
			virtual bool IsSelected(const JUserPtr<Core::JIdentifier>& iden, const UpdateIn& in)const noexcept = 0;
		protected:
			void DoActivate() noexcept override;
			void DoDeActivate() noexcept override;
		};
		class JEditorMouseDragSceneBox final : public JEditorIdentifierDragInterface
		{
		public:
			struct UpdateIn : public JEditorIdentifierDragInterface::UpdateIn
			{
			public:
				JUserPtr<JScene> scene;
				JUserPtr<JCamera> cam;
				J_ACCELERATOR_LAYER layer; 
			public:
				JVector2F sceneImageScreenMinPoint;
			public:
				UpdateIn(const bool allowActivateDragBox,
					JVector2<float> minOffset,
					JVector2<float> maxOffset,
					JVector2<float> sceneImageScreenMinPoint,
					JUserPtr<JScene> scene,
					JUserPtr<JCamera> cam,
					J_ACCELERATOR_LAYER layer);
			}; 
			using UpdateOut = JEditorIdentifierDragInterface::UpdateOut;
		public:
			void UpdateSceneImageDrag(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept;
		private:
			bool IsSelected(const JUserPtr<Core::JIdentifier>& iden, const JEditorIdentifierDragInterface::UpdateIn& in)const noexcept final;
		private:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
		};
		class JEditorMouseDragCanvas final : public JEditorIdentifierDragInterface
		{
		public:
			struct IdenData
			{
			public:
				JUserPtr<Core::JIdentifier> iden;
				Core::JBBox2D bbox;
			public:
				IdenData() = default;
				IdenData(const JUserPtr<Core::JIdentifier>& iden, const Core::JBBox2D& bbox);
			};
		public:
			struct UpdateIn : public JEditorIdentifierDragInterface::UpdateIn
			{
			public: 
				std::vector<IdenData> iden;
				std::unordered_map<size_t, JUserPtr<Core::JIdentifier>> selectedMap;
			public:
				//just draw dragging box or try to activate dragging box
				UpdateIn(const bool allowActivateDragBox,
					JVector2<float> minOffset,
					JVector2<float> maxOffset);
				UpdateIn(const bool allowActivateDragBox,
					JVector2<float> minOffset,
					JVector2<float> maxOffset, 
					std::vector<IdenData>&& iden,
					std::unordered_map<size_t, JUserPtr<Core::JIdentifier>>&& selectedMap);
			}; 
			using UpdateOut = JEditorIdentifierDragInterface::UpdateOut;
		public:
			void UpdateCanvasDrag(_In_ const UpdateIn& in, _Out_ UpdateOut& out)noexcept;
		public:
			/**
			* @brief valid on same vector count
			*/
			static std::vector<IdenData> Combine(const std::vector<JUserPtr<Core::JIdentifier>>& idenVec, const std::vector<Core::JBBox2D>& bboxVec);
		private:
			bool IsSelected(const JUserPtr<Core::JIdentifier>& iden, const JEditorIdentifierDragInterface::UpdateIn& in)const noexcept final;
		private:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
		};
	}
}
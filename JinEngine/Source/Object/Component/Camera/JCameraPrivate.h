#pragma once
#include"JCameraState.h"
#include"../JComponentPrivate.h"
 
namespace JinEngine
{
	namespace Graphic
	{
		struct JCameraConstants;
		struct JDepthTestPassConstants;
		struct JHzbOccComputeConstants;
		struct JSsaoConstants;
		struct JDrawCondition; 
		struct JDrawHelper;
		class JGraphic;
		class JSceneDraw;
		class JHZBOccCulling;
		class JDepthTest;
	}
	namespace Editor
	{
		class JSceneObserver;
	}
	class JCamera;
	class JScene;
	class JFrameDirtyBase;
	class JCameraPrivate final : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface final : public JComponentPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final; 
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept;
		};
		class DestroyInstanceInterface final : public JComponentPrivate::DestroyInstanceInterface
		{
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced)noexcept final;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class Graphic::JGraphic;
		private: 
			static bool UpdateStart(JCamera* cam, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JCamera* cam, Graphic::JCameraConstants& constants)noexcept;
			static void UpdateFrame(JCamera* cam, Graphic::JDepthTestPassConstants& constant)noexcept;
			static void UpdateFrame(JCamera* cam, Graphic::JHzbOccComputeConstants& constant, const uint queryCount, const uint queryOffset)noexcept;
			static void UpdateFrame(JCamera* cam, Graphic::JSsaoConstants& constant)noexcept;
			static void UpdateEnd(JCamera* cam)noexcept; 
		private:
			static int GetCamFrameIndex(JCamera* cam)noexcept;
			static int GetDepthTestPassFrameIndex(JCamera* cam)noexcept;
			static int GetHzbOccComputeFrameIndex(JCamera* cam)noexcept;
			static int GetSsaoFrameIndex(JCamera* cam)noexcept;
		private:
			static bool IsLastFrameHotUpdated(JCamera* cam)noexcept;
			static bool IsLastUpdated(JCamera* cam)noexcept;
			static bool HasCamRecopyRequest(JCamera* cam)noexcept;
			static bool HasDepthTestPassRecopyRequest(JCamera* cam)noexcept; 
			static bool HasHzbOccComputeRecopyRequest(JCamera* cam)noexcept;
			static bool HasSsaoRecopyRequest(JCamera* cam)noexcept;
		};
		class FrameIndexInterface final
		{
		private: 
			friend struct Graphic::JDrawHelper;
		private:
			static int GetCamFrameIndex(JCamera* cam)noexcept;
			static int GetDepthTestPassFrameIndex(JCamera* cam)noexcept;
			static int GetHzbOccComputeFrameIndex(JCamera* cam)noexcept;
			static int GetSsaoFrameIndex(JCamera* cam)noexcept;
		};
		class EditorSettingInterface final
		{
		private:
			friend struct Graphic::JDrawCondition;
			friend class Graphic::JGraphic; 
			friend class Editor::JSceneObserver;
		private:
			static void SetAllowAllCullingResult(const JUserPtr<JCamera>& cam, const bool value)noexcept;
			static bool AllowAllCullingResult(const JUserPtr<JCamera>& cam)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}
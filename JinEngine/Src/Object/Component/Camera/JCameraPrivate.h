#pragma once
#include"JCameraState.h"
#include"../JComponentPrivate.h"
 
namespace JinEngine
{
	namespace Graphic
	{
		struct JCameraConstants;
		class JGraphic;
	}
	namespace Editor
	{
		class JSceneObserver;
	}
	class JCamera;
	class JFrameDirtyBase;
	class JCameraPrivate final : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface final : public JComponentPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		private:
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept;
		};
		class CameraStateInterface final
		{
		private:
			friend class Editor::JSceneObserver;
		private:
			static void SetCameraState(JCamera* cam, const J_CAMERA_STATE state)noexcept;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class Graphic::JGraphic;
		private: 
			static bool UpdateStart(JCamera* cam, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JCamera* cam, Graphic::JCameraConstants& constants)noexcept;
			static void UpdateEnd(JCamera* cam)noexcept;
			static bool IsHotUpdated(JCamera* cam)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}
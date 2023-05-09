#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{
	class JScene;
	class JAnimator;
	namespace Core
	{
		class JGameTimer;
	}
	namespace Graphic
	{
		class JGraphic;
		struct JAnimationConstants;
	}
	class JAnimatorPrivate final : public JComponentPrivate
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
		class AnimationUpdateInterface final
		{
		private:
			friend class JScene;
		private:
			static void OnAnimationUpdate(JUserPtr<JAnimator> ani, Core::JGameTimer* sceneTimer)noexcept;
			static void OffAnimationUpdate(JUserPtr<JAnimator> ani)noexcept;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class Graphic::JGraphic;
		private:
			static bool UpdateStart(JAnimator* ani)noexcept;
			static void UpdateFrame(JAnimator* ani, Graphic::JAnimationConstants& constant)noexcept;
			static void UpdateEnd(JAnimator* ani)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}
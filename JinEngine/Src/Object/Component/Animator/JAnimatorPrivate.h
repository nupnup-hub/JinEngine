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
		class AnimationUpdateInterface final
		{
		private:
			friend class JScene;
		private:
			static void OnAnimationUpdate(JAnimator* ani, Core::JGameTimer* sceneTimer)noexcept;
			static void OffAnimationUpdate(JAnimator* ani)noexcept;
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
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}
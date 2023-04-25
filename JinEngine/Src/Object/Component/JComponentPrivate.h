#pragma once
#include"../JObjectPrivate.h" 

namespace JinEngine
{
	class JGameObject;
	class JGameObjectPrivate;
	class JComponent;  
	class JComponentPrivate : public JObjectPrivate
	{
	public:
		class CreateInstanceInterface : public JObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class Core::JIdenCreatorInterface; 
		private:
			void RegisterCash(Core::JIdentifier* createdPtr)noexcept override; 
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept override;
		}; 
		class DestroyInstanceInterface : public JObjectPrivate::DestroyInstanceInterface
		{
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced) override;
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept override;
			void DeRegisterCash(Core::JIdentifier* ptr)noexcept override;
		};
		class AssetDataIOInterface
		{
		private: 
			friend class JGameObjectPrivate;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(JGameObject* invoker, std::wifstream& stream);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIData(JComponent* comp, std::wofstream& stream);
		private:
			virtual Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) = 0;
			virtual Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) = 0;
		};
		class ActivateInterface final
		{
		private:
			friend class JGameObject;
			friend class JGameObjectPrivate;
		private:
			static void Activate(JComponent* ptr)noexcept;
			static void DeActivate(JComponent* ptr)noexcept;
		};
	public:
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		virtual AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0;
	};
}
/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../JObjectPrivate.h" 

namespace JinEngine
{
	class JGameObject;
	class JGameObjectPrivate;
	class JComponent;  
	class JFileIOTool;
	class JComponentPrivate : public JObjectPrivate
	{
	public:
		class CreateInstanceInterface : public JObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class Core::JIdenCreatorInterface;  
		protected: 
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		private:
			void RegisterCash(Core::JIdentifier* createdPtr)noexcept override; 
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept override;
		}; 
		class DestroyInstanceInterface : public JObjectPrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced) override;
		private:
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept override;
			void DeRegisterCash(Core::JIdentifier* ptr)noexcept override;
		};
		class AssetDataIOInterface
		{
		private: 
			friend class JGameObjectPrivate;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(const JUserPtr<JGameObject>& invoker, JFileIOTool& tool, const size_t typeGuid);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIData(const JUserPtr<JComponent>& comp, JFileIOTool& tool);
		private:
			virtual JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) = 0;
			virtual Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) = 0;
		};
		class ActivateInterface final
		{
		private:
			friend class JGameObject;
			friend class JGameObjectPrivate;
		private:
			static void Activate(const JUserPtr<JComponent>& ptr)noexcept;
			static void DeActivate(const JUserPtr<JComponent>& ptr)noexcept;
		};
	public:
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		virtual AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0;
	};
}
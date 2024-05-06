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
	namespace Editor
	{
		class JWindowDirectory;
	}
	class JResourceManager;
	class JResourceObjectIO;
	class JResourceObject; 
	class JFileIOTool;

	class JDirectoryPrivate final : public JObjectPrivate
	{
	public:
		class CreateInstanceInterface;
		class AssetDataIOInterface final
		{ 
		private:
			friend class JResourceObjectIO;
			friend class CreateInstanceInterface;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(const JUserPtr<JDirectory>& parent, const Core::JAssetFilePathData& pathData);
		private:
			static JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data);
			static Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data);
			static Core::J_FILE_IO_RESULT LoadMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data);	//use initData
			static Core::J_FILE_IO_RESULT StoreMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data);	//use storeData
		};
		class CreateInstanceInterface final : public JObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
		private:
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			void RegisterCash(Core::JIdentifier* createdPtr)noexcept final; 
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept final;
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept;
		};
		class DestroyInstanceInterface final : public JObjectPrivate::DestroyInstanceInterface
		{
		private:
			friend class Core::JIdentifier;
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced) final;  
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept final;
			void DeRegisterCash(Core::JIdentifier* ptr)noexcept final;
		};
		class FileInterface final
		{
		private:
			friend class JResourceObject; 
			friend class JResourceObjectIO;
		private: 
			static void ConvertToActFileData(const JUserPtr<JResourceObject>& rObj) noexcept;
			static void ConvertToDeActFileData(const size_t guid) noexcept;
		private:
			static JUserPtr<JFile> CreateJFile(const JFileInitData& initData, const JUserPtr<JDirectory>& owner);
			static bool DestroyJFile(const size_t rGuid);
		};
		class ActivationInterface final
		{
		private:
			friend class Editor::JWindowDirectory;
			friend class JResourceManager;
		private:
			static void OpenDirectory(const JUserPtr<JDirectory>& dir)noexcept;
			static void CloseDirectory(const JUserPtr<JDirectory>& dir)noexcept;
		};
		class RawDirectoryInterface final
		{
		private:
			friend class Editor::JWindowDirectory; 
		private:
			static void MoveDirectory(const JUserPtr<JDirectory>& dir, const JUserPtr<JDirectory>& newParent)noexcept;
			static void DeleteDirectory(const JUserPtr<JDirectory>& dir)noexcept;
		};
		class DestroyInstanceInterfaceEx
		{
		private:
			friend class JResourceManager;
		private:
			static void BeginForcedDestroy(JDirectory* dir) noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
	};
}

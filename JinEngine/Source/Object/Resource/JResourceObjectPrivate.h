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
#include"JResourceObjectEventType.h"

namespace JinEngine
{ 
	class JDirectory;
	class JFile;
	class JFileIOTool;
	class JResourceObject; 
	class JResourceManager; 
	struct JResourceEventDesc;

	namespace Editor
	{
		class JWindowDirectory;
	}
	namespace Core
	{
		struct JAssetFilePathData;
	}

	class JResourceObjectPrivate : public JObjectPrivate
	{ 
	public:
		class CreateInstanceInterface : public JObjectPrivate::CreateInstanceInterface
		{
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		private:
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept override;
		private:
			//오브젝트를 생성하거나 생성한 후 저장하기위해 캐싱해둔 데이터를 지운다
			virtual void TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept final;
		};
		class DestroyInstanceInterface : public JObjectPrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced) override;
		private:
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept override;
		};
		class AssetDataIOInterface
		{
		private:
			friend class JResourceObject;
			friend class JResourceManager;	//store resource
			friend class JFile;		//load resource 
			friend class CreateInstanceInterface;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(const JUserPtr<JDirectory>& owner, const Core::JAssetFilePathData& pathData);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIDate(const JUserPtr<JResourceObject>& rObj);
		private:
			virtual JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) = 0;
			virtual Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) = 0;
		private:
			virtual Core::J_FILE_IO_RESULT LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data) = 0;	//use initData
			virtual Core::J_FILE_IO_RESULT StoreMetaData(Core::JDITypeDataBase* data) = 0;	//use storeData	 
		protected:
			static Core::J_FILE_IO_RESULT LoadCommonMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data, const bool canClose = false);	//use initData
			static Core::J_FILE_IO_RESULT StoreCommonMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data, const bool canClose = false);	//use storeData	
		}; 
		class FileInterface
		{
		private:
			friend class JDirectory;
			friend class Editor::JWindowDirectory;
		private:
			static JUserPtr<JFile> CopyJFile(const JUserPtr<JResourceObject>& from, const JUserPtr<JDirectory>& toDir, bool setNewInnderGuid = false)noexcept;
		private:
			static void MoveFile(const JUserPtr<JResourceObject>& rObj, const JUserPtr<JDirectory>& toDir)noexcept;
			/**
			* DeleteFile 호출자는 함수호출이후 JResourceObject Destroy에 대한 책임을 진다.
			*/
			static void DeleteFile(JResourceObject* rObj)noexcept;	//delete disk asset file
		};
		class DestroyInstanceInterfaceEx
		{ 
		private:
			friend class JResourceManager; 
		private:
			static void BeginForcedDestroy(JResourceObject* rObj) noexcept;
		};
		class EventInterface
		{
		public:
			static void NotifyEvent(JResourceObject* rObj, const J_RESOURCE_EVENT_TYPE type, std::unique_ptr<JResourceEventDesc>&& desc = nullptr);
		};
	public:
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		virtual JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0;
	};

}
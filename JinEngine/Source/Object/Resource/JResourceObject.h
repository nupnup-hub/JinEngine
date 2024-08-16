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
#include"JResourceObjectType.h"  
#include"JResourceObjectEventType.h" 
#include"JReferenceInterface.h"
#include"../JObject.h"
#include"../GraphicRule/JGraphicModuleManagedDataUser.h"
#include"../../Core/File/JFilePathData.h" 
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Event/JEventManager.h"  

namespace JinEngine
{
	class JDirectory;
	class JFile;
	class JResourceObjectPrivate;
	struct RTypeHint;
	struct RTypeCommonFunc;
	struct RTypePrivateFunc;
	struct JResourceEventDesc;

	//feature
	/*
	* Purpose
	* Engine내부에서만 사용되는 자원 혹은 외부 포맷의 자원들은 모두 .jasset으로 생성 or 변환되어서 저장되며
	* JResourceObject 객체의 하위 class들로서 각각 자원에 대한 접근과 제어를 책임진다.
	*/
	/*
	* Manage
	* RAII을 준수하며 Ref count가 0가 될시 JResourceObject는 파괴되고 대신 JResourceObject에 메타데이터를 소유한
	* JFile이 생성되며 해당 리소스는 파일에서 열람가능한 상태이고 GetResource()를 통한 인스턴스를 요구할시 
	* 다시 생성된다.
	*/
	/* 
	* Cache
	* Editor에서 시뮬레이션등을 수행할때 Resource의 값을 변경하는 경우가 있으며 Rollback을 위해
	* Cache file을 추가하고 Engine이 종료될시 일괄 삭제된다.
	*/
	/*
	* Metafile
	* 경량의 Metafile로 Resource의 특성을 빠른시간에 읽어올 수 있다.
	*/

	class JResourceObject;
	using JResourceEventManager = Core::JEventManager<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*, JResourceEventDesc*>;
	using JResourceEventInterface = JResourceEventManager::Interface;
	class JResourceObject : public JObject, public JReferenceInterface, public Core::JValidInterface, public JGraphicModuleUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JResourceObject)
	public: 
		class InitData : public JObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public: 
			uint8 formatIndex;
			J_RESOURCE_TYPE rType;
			JUserPtr<JDirectory> directory;
		public: 
			InitData(const JTypeInfo& initTypeInfo, 
				const uint8 formatIndex, 
				const J_RESOURCE_TYPE rType, 
				const JUserPtr<JDirectory>& directory);
			InitData(const JTypeInfo& initTypeInfo,
				const size_t guid,
				const uint8 formatIndex,
				const J_RESOURCE_TYPE rType,
				const JUserPtr<JDirectory>& directory);
			InitData(const JTypeInfo& initTypeInfo, 
				const std::wstring& name,
				const size_t& guid,
				const J_OBJECT_FLAG flag, 
				const uint8 formatIndex,
				const J_RESOURCE_TYPE rType,
				const JUserPtr<JDirectory>& directory);
		public:
			bool IsValidData()const noexcept override;
			J_RESOURCE_TYPE GetResourceType() const noexcept;
			std::wstring GetFormat()const noexcept;
		};
	protected: 
		class LoadData final : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadData)
		public:
			JUserPtr<JDirectory> directory;
			const Core::JAssetFilePathData pathData;
		public:
			LoadData(const JUserPtr<JDirectory>& directory, const Core::JAssetFilePathData& pathData);
			~LoadData();
		public:
			bool IsValidData()const noexcept final;
		};   
	protected: 
		class StoreData final : public JObject::StoreData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)  
		public:
			StoreData(const JUserPtr<JResourceObject>& jRobj);
		};
	private:
		friend class JResourceObjectPrivate;
		class JResourceObjectImpl;
	private:
		std::unique_ptr<JResourceObjectImpl> impl;
	public:
		static constexpr uint8 GetInvalidFormatIndex()noexcept
		{
			return 255;
		}
		static constexpr uint8 GetDefaultFormatIndex()noexcept
		{
			return 0;
		}
	public:
		//Return name + oriFormat
		std::wstring GetFullName()const noexcept; 
		std::wstring GetPath()const noexcept;  
		std::wstring GetMetaFilePath()const noexcept;
		std::wstring GetFolderPath()const noexcept;  
		static std::wstring GetDefaultFormat(const J_RESOURCE_TYPE type)noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		uint8 GetFormatIndex()const noexcept;
		static uint8 GetFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)noexcept;
		JUserPtr<JDirectory> GetDirectory()const noexcept;
		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual std::wstring GetFormat()const noexcept = 0; 
	public:
		void SetName(const std::wstring& newName)noexcept final;
	protected:
		bool HasFile()const noexcept;
		bool HasMetafile()const noexcept; 
		bool CanMakeFile()const noexcept;	//Can make diskfile && JFile
		static bool IsResourceFormat(const J_RESOURCE_TYPE type, const std::wstring& format)noexcept;
	protected:
		//DoActivate => Load resource file in memory
		void DoActivate() noexcept override;
		//DoDeActivate => Unload resource
		void DoDeActivate()noexcept override;
	public:
		static JResourceEventInterface* EvInterface()noexcept;
	protected:
		static void RegisterRTypeInfo(const Core::JTypeInfo& typeInfo, const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypePrivateFunc& rTypePFunc);
	public:
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		static std::wstring GetDefaultFormat()noexcept
		{
			return GetDefaultFormat(T::GetStaticResourceType());
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		static uint8 GetFormatIndex(const std::wstring& format)noexcept
		{
			return GetFormatIndex(T::GetStaticResourceType(), format);
		}
	protected:
		JResourceObject(const InitData& initData);
		~JResourceObject();
	};
}
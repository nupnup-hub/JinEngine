#pragma once
#include"JResourceObjectInterface.h"
#include"JResourceType.h" 
#include"JResourcePathData.h" 
#include"../../Core/File/JFilePathData.h"
#include"../../Utility/JTypeUtility.h"

namespace JinEngine
{
	class JDirectory;
	class JResourceObject : public JResourceObjectInterface
	{
		REGISTER_CLASS(JResourceObject) 
	protected:
		struct JResourceMetaData : public JObjectMetaData
		{
		public:
			std::wstring format;
			int formatIndex; 
			int rType;
		};
	public:
		struct JResourceInitData
		{ 
		public:
			const std::wstring name;
			const size_t guid;
			const J_OBJECT_FLAG flag;
			const uint8 formatIndex;
			const J_RESOURCE_TYPE rType;
		private:
			JDirectory* directory;
		public:
			JResourceInitData(const std::wstring& name,
				const size_t guid, 
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex,
				const J_RESOURCE_TYPE rType);
			JResourceInitData(const std::wstring& name,
				JDirectory* directory,
				const uint8 formatIndex,
				const J_RESOURCE_TYPE rType);
			virtual ~JResourceInitData() = default;
		public:
			JDirectory* GetDirectory()const noexcept;
		public:
			virtual bool IsValidCreateData();
			virtual bool IsValidLoadData();
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		};
	private:
		//0 is default Resource format 
		const uint8 formatIndex; 
		JDirectory* directory;  
	public:
		//Return name + oriFormat
		std::wstring GetFullName()const noexcept; 
		std::wstring GetPath()const noexcept;  
		std::wstring GetMetafilePath()const noexcept;
		std::wstring GetFolderPath()const noexcept;  
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual std::wstring GetFormat()const noexcept = 0; 
		void SetName(const std::wstring& newName)noexcept final;
	protected:
		uint8 GetFormatIndex()const noexcept;
		JDirectory* GetDirectory()const noexcept final;
		bool HasFile()const noexcept;
		bool HasMetafile()const noexcept;
	protected:
		bool CopyRFile(JResourceObject& ori, bool setNewInnderGuid = false); 
	protected:
		//DoActivate => Load resource file in memory
		void DoActivate() noexcept override;
		//DoDeActivate => Unload resource
		void DoDeActivate()noexcept override;
	private: 
		void OnReference()noexcept final;
		void OffReference()noexcept final;
	protected:
		bool Destroy(const bool isForced)override;
		void DeleteRFile()final;
		void MoveRFile(JDirectory* newDir)final;
	private:
		bool RegisterCashData()noexcept final;
		bool DeRegisterCashData()noexcept final;
	protected:
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JResourceObject* rObject);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, JResourceMetaData& metadata);
	protected:
		JResourceObject(const JResourceInitData& initdata);
		~JResourceObject();
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
		template<typename T>
		static auto Convert(JObject* obj) -> typename Core::TypeCondition<T*, std::is_base_of_v<JResourceObject, T>>::Type
		{
			if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				JResourceObject* jRobj = static_cast<JResourceObject*>(obj);
				if (jRobj->GetResourceType() == T::GetStaticResourceType())
					return static_cast<T*>(jRobj);
			}
			return nullptr;
		}
	protected: 
		template<typename T>
		static uint8 GetFormatIndex(const std::wstring& format)noexcept
		{
			std::vector<std::wstring> formatVec = T::GetAvailableFormat();
			const uint formatVecCount = (uint)formatVec.size();
			for (uint i = 0; i < formatVecCount; ++i)
			{
				if (formatVec[i] == format)
					return i;
			}
			return GetInvalidFormatIndex();
		}
		template<typename T>
		static bool IsResourceFormat(const std::wstring& format)noexcept
		{
			std::vector<std::wstring> formatVec = T::GetAvailableFormat();
			const uint formatVecCount = (uint)formatVec.size();
			for (uint i = 0; i < formatVecCount; ++i)
			{
				if (formatVec[i] == format)
					return true;
			}
			return false;
		}
	};
}
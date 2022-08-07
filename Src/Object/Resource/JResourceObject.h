#pragma once
#include"JResourceObjectInterface.h"
#include"JResourceType.h" 
#include"JResourcePathData.h"
#include<vector>
#include<fstream>

namespace JinEngine
{
	class JDirectory;
	class JResourceObject : public JResourceObjectInterface
	{
		REGISTER_CLASS(JResourceObject) 
	protected:
		//0 is default Resource format
		const uint8 formatIndex;
	private: 
		JDirectory* directory;  
	public:
		std::string GetPath()const noexcept;
		std::wstring GetWPath()const noexcept;
		std::string GetFolderPath()const noexcept; 
		std::wstring GetFolderWPath()const noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual std::string GetFormat()const noexcept = 0; 
	protected:
		std::string GetMetafilePath()const noexcept;
		JDirectory* GetDirectory()noexcept;   
		JResourceObject* CopyResource();
	protected:
		static std::wstring ConvertMetafilePath(const std::wstring& resourcePath)noexcept; 
		static bool HasMetafile(const std::string& path);
		static bool HasMetafile(JResourceObject* resource);
	private: 
		virtual Core::J_FILE_IO_RESULT CallStoreResource() = 0;
	protected:
		void DoActivate() noexcept override;
		void DoDeActivate()noexcept override;
	private: 
		void OnReference()noexcept final;
		void OffReference()noexcept final;
	protected:
		JResourceObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
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
	protected: 
		template<typename T>
		static int GetFormatIndex(const std::string& format)noexcept
		{
			std::vector<std::string> formatVec = T::GetAvailableFormat();
			const uint formatVecCount = (uint)formatVec.size();
			for (uint i = 0; i < formatVecCount; ++i)
			{
				if (formatVec[i] == format)
					return i;
			}
			return -1;
		}
		template<typename T>
		static bool IsResourceFormat(const std::string& format)noexcept
		{
			std::vector<std::string> formatVec = T::GetAvailableFormat();
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
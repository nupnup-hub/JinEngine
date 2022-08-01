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
		const uint8 formatIndex;
	private: 
		JDirectory* directory;
		int referenceCount;
	public:
		JResourceObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JResourceObject(); 
		JResourceObject(JResourceObject&& rhs) = default;
		JResourceObject& operator=(JResourceObject&& rhs) = default;

		std::string GetPath()const noexcept;
		std::wstring GetWPath()const noexcept;
		std::string GetFolderPath(bool eraseLastBackSlash)const noexcept; 
		int GetReferenceCount()const noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual std::string GetFormat()const noexcept = 0; 

		void OnReference()noexcept;
		void OffReference()noexcept; 
	protected:
		void DoActivate() noexcept override;
		void DoDeActivate()noexcept override;
		std::string GetMetafilePath()const noexcept;
		JDirectory* GetDirectory()noexcept;  
		JResourceObject* CopyResource();

		static std::wstring ConvertMetafilePath(const std::wstring& resourcePath)noexcept; 
		static bool HasMetafile(const std::string& path);
		static bool HasMetafile(JResourceObject* resource);
	private: 
		virtual Core::J_FILE_IO_RESULT CallStoreResource() = 0;
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
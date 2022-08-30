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
	private:
		//0 is default Resource format 
		const uint8 formatIndex; 
		JDirectory* directory;  
	public:
		std::wstring GetFullName()const noexcept; 
		std::wstring GetPath()const noexcept;
		std::wstring GetFolderPath()const noexcept;  
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual std::wstring GetFormat()const noexcept = 0; 
	protected:
		uint8 GetFormatIndex()const noexcept;
		std::wstring GetMetafilePath()const noexcept;
		JDirectory* GetDirectory()noexcept;    
	protected:
		static std::wstring ConvertMetafilePath(const std::wstring& resourcePath)noexcept; 
		static bool HasMetafile(const std::wstring& path);
		static bool HasMetafile(JResourceObject* resource);
	protected:
		//DoActivate => Load resource file in memory
		void DoActivate() noexcept override;
		//DoDeActivate => Unload resource
		void DoDeActivate()noexcept override;
	private: 
		void OnReference()noexcept final;
		void OffReference()noexcept final;
	protected:
		void Destroy()override; 
	protected:
		static void CopyRFile(JResourceObject& from, JResourceObject& to);
	protected:
		JResourceObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
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
		static int GetFormatIndex(const std::wstring& format)noexcept
		{
			std::vector<std::wstring> formatVec = T::GetAvailableFormat();
			const uint formatVecCount = (uint)formatVec.size();
			for (uint i = 0; i < formatVecCount; ++i)
			{
				if (formatVec[i] == format)
					return i;
			}
			return -1;
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
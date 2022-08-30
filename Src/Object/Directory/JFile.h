#pragma once
#include<string>  

namespace JinEngine
{
	class JResourceObject;  
	class JDirectory;
	class JFile
	{   
	private:
		friend class JDirectory;
	private:
		JResourceObject* resource; 
	public:
		std::wstring GetName()const noexcept;
		std::wstring GetFullName()const noexcept;
		std::wstring GetAvailableFormat()const noexcept;
		JResourceObject* GetResource()noexcept;  
	private:
		JFile(JResourceObject* resource);
		~JFile() = default;
		//JFile(JFile && rhs) = default;
		//JFile& operator=(JFile&& rhs) = default;
	};
}
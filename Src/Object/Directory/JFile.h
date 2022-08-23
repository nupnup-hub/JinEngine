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
		std::string GetName()const noexcept;
		std::string GetFullName()const noexcept;
		std::string GetAvailableFormat()const noexcept;
		JResourceObject* GetResource()noexcept;  
	private:
		JFile(JResourceObject* resource);
		~JFile() = default;
		//JFile(JFile && rhs) = default;
		//JFile& operator=(JFile&& rhs) = default;
	};
}
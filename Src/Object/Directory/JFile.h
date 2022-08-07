#pragma once
#include<string> 

namespace JinEngine
{
	class JResourceObject; 
	class JFile
	{  
	private:
		JResourceObject* resource; 
	public:
		JFile(JResourceObject* resource);
		~JFile();
		JFile(JFile&& rhs) = default;
		JFile& operator=(JFile&& rhs) = default;
		 
		std::string GetName()const noexcept;
		std::string GetAvailableFormat()const noexcept;
		JResourceObject* GetResource()noexcept;  
	};
}
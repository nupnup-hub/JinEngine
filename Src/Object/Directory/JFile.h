#pragma once
#include<string> 

namespace JinEngine
{
	class JResourceObject;
	class PreviewScene;
	class JFile
	{  
	private:
		JResourceObject* resource;
		PreviewScene* previewScene;
	public:
		JFile(JResourceObject* resource);
		~JFile();
		JFile(JFile&& rhs) = default;
		JFile& operator=(JFile&& rhs) = default;
		 
		std::string GetName()const noexcept;
		std::string GetAvailableFormat()const noexcept;
		JResourceObject* GetResource()noexcept;
		PreviewScene* GetPreviewScene()noexcept;
		void SetPreviewScene(PreviewScene* previewScene)noexcept; 

	};
}
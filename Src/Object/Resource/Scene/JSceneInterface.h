#pragma once
#include"../JResourceObject.h" 

namespace JinEngine
{
	class PreviewResourceScene;
	class JSceneInterface : public JResourceObject
	{
	private:
		friend class PreviewResourceScene;
	public:
		JSceneInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	private:
		virtual void MakeDefaultObject(bool isEditorScene)noexcept = 0;
	};
}
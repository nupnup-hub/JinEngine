#pragma once 
#include"../JResourceObject.h"
#include"../JClearableInterface.h" 
#include"../JResourceUserInterface.h"

namespace JinEngine
{ 
	class JScene;
	class JGameObject;
	class JPreviewModelScene;
	namespace Editor
	{
		class JEditorSkeletonPage;
		class JAvatarEditor; 
	}

	class JModelSceneInterface 
	{
	private:
		friend class JPreviewModelScene;
		friend class Editor::JEditorSkeletonPage;
		friend class Editor::JAvatarEditor;
	protected:
		virtual ~JModelSceneInterface() = default;
	public:
		virtual JModelSceneInterface* ModelSceneInterface() = 0;
	private:
		virtual JScene* GetModelScene()noexcept = 0;
		virtual JGameObject* GetModelRoot()noexcept = 0;
		virtual JGameObject* GetSkeletonRoot()noexcept = 0;
	};

	class JModelInterface : public JResourceObject,
		public JModelSceneInterface,
		public JClearableInterface, 
		public JResourceUserInterface
	{
	protected:
		JModelInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}
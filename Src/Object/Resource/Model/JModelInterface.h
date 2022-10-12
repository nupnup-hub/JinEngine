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
	private:
		virtual JScene* GetModelScene()noexcept = 0;
		virtual JGameObject* GetModelRoot()noexcept = 0;
		virtual JGameObject* GetSkeletonRoot()noexcept = 0;
	};

	struct JModelAttribute;
	struct JModelPart;
	struct Joint;
	namespace Core
	{
		struct JObjMeshPartData;
		struct JObjMatData;
		struct JFbxPartMeshData;
	}
	class JModelImportInterface
	{
	private:
		friend class JResourceManagerImpl;
	private:
		virtual bool ImportObject(const std::vector<Core::JObjMeshPartData>& objMeshData,
			const std::vector<Core::JObjMatData>& objMatData,
			const JModelAttribute& attribute)noexcept = 0;
		virtual bool ImportObject(const std::vector<Core::JFbxPartMeshData>& jFbxPartMeshData,
			const JModelAttribute& modelAttribute,
			const std::vector<Joint>& joint)noexcept = 0;
	};


	class JModelInterface : public JResourceObject,
		public JClearableInterface, 
		public JResourceUserInterface,
		public JModelSceneInterface,
		public JModelImportInterface
	{
	protected:
		JModelInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}
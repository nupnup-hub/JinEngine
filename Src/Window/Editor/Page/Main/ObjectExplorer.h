#pragma once
#include"../EditorWindow.h" 
#include"../../Event/EditorEventType.h"
#include"../../Event/EditorEventStruct.h" 
#include"../../../../Object/JObjectType.h"
#include"../../../../Utility/JDelegate.h" 
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
 
namespace JinEngine
{
	class JGameObject;
	class JObject;
	class EditorString;
	class EditorPopup;
	enum class OBJECT_EXPLORER_EVENT_TYPE
	{
		NONE,
		CHANGE_PARENT,
		MAKE_MODEL
	};
	struct ObjectExplorerEvent
	{
	public:
		OBJECT_EXPLORER_EVENT_TYPE eventType = OBJECT_EXPLORER_EVENT_TYPE::NONE;
		JGameObject* selectGameObj = nullptr;
		JObject* tarObject = nullptr;
	};

	class ObjectExplorer : public EditorWindow, public Core::JEventListener<size_t, EDITOR_EVENT, EditorEventStruct*>
	{
		struct JDelegateDataStruct
		{
		public: 
			bool activatedRename;
			size_t renameObjectGuid;
			std::string renameBuf;
		};
	private: 
		std::unique_ptr<EditorString>editorString;
		std::unique_ptr<EditorPopup>explorerPopup;
		JDelegateDataStruct JDelegateData;
		std::unordered_map<size_t, JDelegate<void(EditorUtility*)>> gameObjFunc;
	public:
		ObjectExplorer(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~ObjectExplorer();
		ObjectExplorer(const ObjectExplorer& rhs) = delete;
		ObjectExplorer& operator=(const ObjectExplorer& rhs) = delete;

		void Initialize(EditorUtility* editorUtility)noexcept;
		void UpdateWindow(EditorUtility* editorUtility)override;
		 
		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final;
		bool OnFocus(EditorUtility* editorUtility) final;
		bool OffFocus(EditorUtility* editorUtility) final;
	private:
		void BuildObjectExplorer(JScene* scene, EditorUtility* editorUtility);
		void ObjectExplorerOnScreen(JScene* scene, JGameObject* obj, EditorUtility* editorUtility, ObjectExplorerEvent& objectExplorerEvent);
	private:
		virtual void OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)final;
	};
}
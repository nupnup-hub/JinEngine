#pragma once  
#include<vector> 
#include<string>
#include<unordered_map>
#include"../Event/EditorEventType.h"   
#include"../Event/EditorEventStruct.h"   
#include"../../../Core/JDataType.h"  
#include"../../../Core/Event/JEventManager.h"
#include"../../../Core/Empty/EmptyBase.h"

namespace JinEngine
{ 
	class JObject;  
	class EditorUtility : public Core::JEventManager<Core::EmptyBase, size_t, EDITOR_EVENT, void, EditorEventStruct*>
	{
	public:
		JObject* selectedObject; 

		int displayWidth;
		int displayHeight;
		int clientPositionX;
		int clientPositionY;
		int clientWidth; 
		int clientHeight;
  
		float textWidth;
		float textHeight; 

		bool leftMouseClick;
		bool rightMouseClick;
		bool isDrag; 	  
	public: 
		EditorUtility();
		~EditorUtility(); 

		JEventInterface* EvInterface()noexcept final;

		void UpdateWindoeData();
		void UpdateEditorTextSize();
	};
}
#pragma once 
#include"../EditorWindow.h"
#include"SkeletonAssetSettingPageShareData.h"
#include"../../../../Object/Resource/Skeleton/Avatar/JAvatar.h"
#include"../../../../Utility/Vector.h"
#include<vector>

namespace JinEngine
{ 
	class JGameObject;
	class AvatarEditor : public EditorWindow
	{
	private: 
		SkeletonAssetSettingPageShareData* pageShareData;
		JAvatar targetAvatar; 
		 
		bool hasAvatar = false;
		bool isOpenAvatarSetting = false;
		bool isOpenJointSelector = false;
		int selectedJointGroupNumber;
		int selectJointRefNumber;
		std::string selectJointName;
		std::vector<bool> jointChecker;
		std::vector<bool> tabs{ false,false ,false ,false };
		const Vector4<float> failColor{ 0.85f, 0.2f, 0.2f, 0.75f};
	public:
		AvatarEditor(std::unique_ptr<EditorAttribute> attribute,
			const size_t ownerPageGuid,
			SkeletonAssetSettingPageShareData* pageShareData);
		~AvatarEditor() = default;
		AvatarEditor(const AvatarEditor& rhs) = delete;
		AvatarEditor& operator=(const AvatarEditor& rhs) = delete;

		void Initialize(EditorUtility* editorUtility)noexcept; 
		bool Activate(EditorUtility* editorUtility)override;
		void UpdateWindow(EditorUtility* editorUtility)override;
	private:
		void BuildAvatarEdit(EditorUtility* editorUtility);
		void CloseAllTab();
		void BuildObjectExplorer(EditorUtility* editorUtility);
		void ObjectExplorerOnScreen(JGameObject* obj, EditorUtility* editorUtility);
		void StuffSelectedJointReference(JGameObject* obj);
		void StuffAllJointReferenceByAuto(); 
		void StoreAvatarData();
		//Reference배열에 있는 모든 데이터에 -1대입
		void ClearAllJointReference();
		bool CheckAllJoint(); 
		bool IsAllJointPassed()noexcept;
		bool ArrowClick(const std::string& name);
	};
}
#pragma once 
#include"../EditorWindow.h"
#include"../../Event/EditorEventType.h"
#include"../../Event/EditorEventStruct.h" 
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Object/JObjectType.h"

namespace JinEngine
{
	class JAnimationClip;
	class JMeshGeometry;
	class JMaterial;
	class JModel;
	class JTexture;
	class JSkeletonAsset;
	class StandardMaterial;
	class CubeMapMaterial;
	class EditorObjectPositionCalculator;
	class EditorString;
	class PreviewSceneGroup;
	class JResourceObject;
	class JGameObject;
	class JAnimator;
	class JLight;
	class JCamera;

	struct TransformDetail
	{
		float positionBuff[3] = { 0,0,0 };
		float rotationBuff[3] = { 0,0,0 };
		float scaleBuff[3] = { 0,0,0 };
		float* valueBuffer[3]
		{
			positionBuff, rotationBuff, scaleBuff
		};

		std::string guideText[3]
		{
			"Position", "Rotation", "Scale"
		};
	};
	struct AnimatorDetail
	{ 
		size_t animatorBtnId;
		size_t animatorWindowId;
		size_t skeletonBtnId;
		size_t skeletonWindowId;
	};
	struct CameraDetail
	{ 
	};
	struct LightDetail
	{ 
	};
	struct ComponentSelectorDetail
	{
		size_t componentBtnId;
		size_t componentListboxId;
	};
	struct MeshDetail
	{
		size_t meshBtnId;
		size_t meshWindowId;
	};
	struct MaterialDetail
	{
		float albedoColorBuff[4] = { 0,0,0,0 };
		float materialTransformVectorXBuff[3] = { 1,0,0 };
		float materialTransformVectorYBuff[3] = { 0,1,0 };
		float materialTransformVectorZBuff[3] = { 0,0,1 };
		float metalicBuff = 0;
		float roughnessBuff = 0;

		size_t materialBtnId;
		size_t materialWindowId;
		size_t textureWindowId;
	};

	class ObjectDetail : public EditorWindow, public Core::JEventListener<size_t, EDITOR_EVENT, EditorEventStruct*>
	{
		enum class SELECTOR_STATE
		{
			CLOSE = 0,
			OPEN_MESH,
			OPEN_MATERIAL,
			OPEN_TEXTURE_ALBEDO,
			OPEN_TEXTURE_NORMAL,
			OPEN_TEXTURE_HEIGHT,
			OPEN_TEXTURE_ROUGHNESS,
			OPEN_TEXTURE_AMBIENT,
			OPEN_ANIMATION_CONTROLLER,
			OPEN_COMPONENT,
			OPEN_SKELETON,
		};
 
	private:
		std::unique_ptr<EditorString> editorString;
		std::unique_ptr<EditorObjectPositionCalculator> editorPositionCal;
		static constexpr int previewCapacity = 100;
		const size_t previewGuid;
		PreviewSceneGroup* previewGroup;

		float windowWidth;
		float windowHeight;
		static constexpr float widgetWidthRate = 1;
		static constexpr float widgetHeightRate = 0.025f;
		static constexpr float selectorWindowWidthRate = 0.25f;
		static constexpr float selectorWindowHeightRate = 0.5f;
		static constexpr float selectorIconMaxRate = 0.075f;
		static constexpr float selectorIconMinRate = 0.035f;

		float selectorIconMinSize;
		float selectorIconMaxSize;
		float selectorIconSize = 0; 
		SELECTOR_STATE nowSelectorState = SELECTOR_STATE::CLOSE;
		std::vector<SELECTOR_STATE> selectorEvVec;	
		bool selectorWindowKey = false;	 
		size_t selectorIconSlidebarId;

		TransformDetail transformDetail;
		AnimatorDetail animatiorDetail;
		CameraDetail cameraDetail;
		LightDetail lightDetail;
		ComponentSelectorDetail componentSelectorDetail;
		MeshDetail meshDetail;
		MaterialDetail materialDetail;
	public:
		ObjectDetail(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~ObjectDetail() = default;
		ObjectDetail(const ObjectDetail& rhs) = delete;
		ObjectDetail& operator=(const ObjectDetail& rhs) = delete;

		void Initialize(EditorUtility* editorUtility)noexcept;
		void UpdateWindow(EditorUtility* editorUtility)override;
	 
		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final;
		bool OnFocus(EditorUtility* editorUtility) final;
		bool OffFocus(EditorUtility* editorUtility) final;
	private:
		void SelectedObjectDetailOnScreen(EditorUtility* editorUtility);
		void TransformDetailOnScreen(EditorUtility* editorUtility, JGameObject* gameObj);
		void AnimatorDetailOnScreen(EditorUtility* editorUtility, JAnimator* animator);
		void CameraDetailOnScreen(EditorUtility* editorUtility, JCamera* camera);
		void LightDetailOnScreen(EditorUtility* editorUtility, JLight* light);
		void ShowComponentSelector(EditorUtility* editorUtility, JGameObject* gameObj);

		void MeshDetailOnScreen(EditorUtility* editorUtility, JMeshGeometry* mesh, const J_OBJECT_TYPE objType);
		void MaterialDetailOnScreen(EditorUtility* editorUtility, JMaterial* material, const J_OBJECT_TYPE objType);
		void ModelDataOnScreen(EditorUtility* editorUtility, JModel* model, const J_OBJECT_TYPE objType);
		void AnimationClipDataOnScreen(EditorUtility* editorUtility, JAnimationClip* animationClip, const J_OBJECT_TYPE objType);
		void TextureDataOnScreen(EditorUtility* editorUtility, JTexture* texture, const J_OBJECT_TYPE objType);
		void SkeletonAssetDataOnScreen(EditorUtility* editorUtility, JSkeletonAsset* skeletonAsset, const J_OBJECT_TYPE objType);
		bool ResourceSelectorOnScreen(const std::string& windowName, _In_ EditorUtility* editorUtility, _Out_ JResourceObject** select);
		
		void SearchSelectorEvVec(); 
		bool OpenSelectorWindow(const SELECTOR_STATE selectorState);
		void Spacing();
	private:
		virtual void OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)final;
	};
}

#pragma once 
#include"../../JEditorWindow.h"  
#include"../../../../Object/JObjectType.h"

namespace JinEngine
{
	class JAnimationClip;
	class JMeshGeometry;
	class JMaterial;
	class JModel;
	class JTexture;
	class JSkeletonAsset;
	class JPreviewSceneGroup;
	class JResourceObject;
	class JGameObject;
	class JAnimator;
	class JLight;
	class JCamera;

	namespace Editor
	{
		class JEditorWidgetPosCalculator;
		class JEditorString;
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

		class JObjectDetail : public JEditorWindow
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
			std::unique_ptr<JEditorString> editorString;
			std::unique_ptr<JEditorWidgetPosCalculator> editorPositionCal;
			static constexpr int previewCapacity = 100;
			const size_t previewGuid;
			JPreviewSceneGroup* previewGroup;

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
			ComponentSelectorDetail componentSelectorDetail;
			MeshDetail meshDetail;
			MaterialDetail materialDetail;
		public:
			JObjectDetail(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JObjectDetail() = default;
			JObjectDetail(const JObjectDetail& rhs) = delete;
			JObjectDetail& operator=(const JObjectDetail& rhs) = delete;

			void Initialize(JEditorUtility* editorUtility)noexcept;
			void UpdateWindow(JEditorUtility* editorUtility)override;

			bool Activate(JEditorUtility* editorUtility) final;
			bool DeActivate(JEditorUtility* editorUtility) final;
			bool OnFocus(JEditorUtility* editorUtility) final;
			bool OffFocus(JEditorUtility* editorUtility) final;
		private:
			void SelectedObjectDetailOnScreen(JEditorUtility* editorUtility);
			void TransformDetailOnScreen(JEditorUtility* editorUtility, JGameObject* gameObj);
			void AnimatorDetailOnScreen(JEditorUtility* editorUtility, JAnimator* animator);
			void CameraDetailOnScreen(JEditorUtility* editorUtility, JCamera* camera);
			void LightDetailOnScreen(JEditorUtility* editorUtility, JLight* light);
			void ShowComponentSelector(JEditorUtility* editorUtility, JGameObject* gameObj);

			void MeshDetailOnScreen(JEditorUtility* editorUtility, JMeshGeometry* mesh, const J_OBJECT_TYPE objType);
			void MaterialDetailOnScreen(JEditorUtility* editorUtility, JMaterial* material, const J_OBJECT_TYPE objType);
			void ModelDataOnScreen(JEditorUtility* editorUtility, JModel* model, const J_OBJECT_TYPE objType);
			void AnimationClipDataOnScreen(JEditorUtility* editorUtility, JAnimationClip* animationClip, const J_OBJECT_TYPE objType);
			void TextureDataOnScreen(JEditorUtility* editorUtility, JTexture* texture, const J_OBJECT_TYPE objType);
			void SkeletonAssetDataOnScreen(JEditorUtility* editorUtility, JSkeletonAsset* skeletonAsset, const J_OBJECT_TYPE objType);
			bool ResourceSelectorOnScreen(const std::string& windowName, _In_ JEditorUtility* editorUtility, _Out_ JResourceObject** select);

			void SearchSelectorEvVec();
			bool OpenSelectorWindow(const SELECTOR_STATE selectorState);
			void Spacing();
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)final;
		};
	}
}
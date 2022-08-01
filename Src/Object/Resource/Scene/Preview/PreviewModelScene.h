#pragma once
#include"PreviewScene.h"

namespace JinEngine
{
	class JGameObject;
	class JModel;
	class JScene;

	class PreviewModelScene : public PreviewScene
	{
	private:
		JModel* model;
		JScene* modelScene;
	public:
		PreviewModelScene(const std::string& previewSceneName, _In_ JResourceObject* model, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag);
		~PreviewModelScene();
		PreviewModelScene(PreviewModelScene&& rhs) = default;
		PreviewModelScene& operator=(PreviewModelScene&& rhs) = default;

		bool Initialze()noexcept; 
		void Clear()noexcept final;
	protected:
		JScene* GetScene()noexcept override;
	};
}

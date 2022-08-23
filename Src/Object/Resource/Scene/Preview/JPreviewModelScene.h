#pragma once
#include"JPreviewScene.h"

namespace JinEngine
{
	class JGameObject;
	class JModel;
	class JScene;

	class JPreviewModelScene : public JPreviewScene
	{
	private:
		JModel* model;
		JScene* modelScene;
	public:
		JPreviewModelScene(_In_ JResourceObject* model, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewModelScene();
		JPreviewModelScene(JPreviewModelScene&& rhs) = default;
		JPreviewModelScene& operator=(JPreviewModelScene && rhs) = default;
	public:
		bool Initialze()noexcept; 
		void Clear()noexcept final;
	protected:
		JScene* GetScene()noexcept override;
	};
}

#pragma once

namespace JinEngine
{ 
	class ISceneAnimator abstract
	{
		friend class SceneViewer;
	public:
		virtual ~ISceneAnimator() = default;
	private:
		virtual void ActivateAnimtor()noexcept = 0;
	};
}
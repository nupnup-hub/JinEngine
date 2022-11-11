#pragma once
#include"JCameraState.h" 
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../../Graphic/JGraphicTexture.h"

namespace JinEngine
{ 
	namespace Graphic
	{
		struct JCameraConstants;
	}
	namespace Editor
	{
		class JSceneObserver;
	}

	class JCameraStateInterface
	{
	private: 
		friend class Editor::JSceneObserver;
	protected:
		virtual ~JCameraStateInterface() = default;
	public:
		virtual JCameraStateInterface* StateInterface() = 0;
	private:
		virtual void SetCameraState(const J_CAMERA_STATE state)noexcept = 0;
	};
	 
	class JCameraInterface : public JComponent,
		public JCameraStateInterface,
		public JFrameUpdate<IFrameUpdate<Graphic::JCameraConstants&>, JFrameDirtyListener, false>,
		public Graphic::JGraphicTexture
	{
	protected:
		JCameraInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}
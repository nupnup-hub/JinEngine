#pragma once
#include"JCameraState.h" 
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceHandleInterface.h"

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
		public JFrameUpdate<IFrameUpdate1<IFrameUpdateBase<Graphic::JCameraConstants&>>, JFrameDirtyListener, FrameUpdate::nonBuff>,
		public Graphic::JGraphicResourceHandleInterface
	{
	protected:
		JCameraInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}
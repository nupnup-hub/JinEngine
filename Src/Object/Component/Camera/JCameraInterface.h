#pragma once
#include"JCameraState.h" 
#include"../JComponent.h" 
#include"../../JFrameInterface.h"
#include"../../../Graphic/JGraphicTexture.h"

namespace JinEngine
{
	class JResourceManager;
	class PreviewResourceScene;
	class PreviewModelScene;
	namespace Graphic
	{
		struct JCameraConstants;
	}

	class JCameraStateInterface : public JComponent
	{
	private:
		friend class JResourceManager;
		friend class PreviewResourceScene;
		friend class PreviewModelScene;
	protected:
		JCameraStateInterface(const std::string& cTypeName, size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner);
	public:
		virtual JCameraStateInterface* StateInterface() = 0;
	private:
		virtual void SetCameraState(const J_CAMERA_STATE state)noexcept = 0;
	};

	class JCameraInterface : public JCameraStateInterface, public JFrameInterface<Graphic::JCameraConstants>, public Graphic::JGraphicTexture
	{
	protected:
		JCameraInterface(const std::string& cTypeName, size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner); 
	};
}
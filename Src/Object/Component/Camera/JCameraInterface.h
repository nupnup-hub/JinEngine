#pragma once
#include"JCameraState.h" 
#include"../JComponent.h" 
#include"../../JFrameInterface.h"
#include"../../../Graphic/JGraphicTexture.h"

namespace JinEngine
{
	class JResourceManagerImpl;
	class JPreviewResourceScene;
	class JPreviewModelScene;
	namespace Graphic
	{
		struct JCameraConstants;
	}

	class JCameraStateInterface
	{
	private:
		friend class JResourceManagerImpl;
		friend class JPreviewResourceScene;
		friend class JPreviewModelScene;
	protected:
		virtual ~JCameraStateInterface() = default;
	public:
		virtual JCameraStateInterface* StateInterface() = 0;
	private:
		virtual void SetCameraState(const J_CAMERA_STATE state)noexcept = 0;
	};

	class JCameraInterface : public JComponent,
		public JCameraStateInterface,
		public JFrameInterface<Graphic::JCameraConstants>,
		public Graphic::JGraphicTexture
	{
	protected:
		JCameraInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}
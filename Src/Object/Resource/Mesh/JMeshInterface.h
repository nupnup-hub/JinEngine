#pragma once 
#include"../JResourceObject.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	struct JStaticMeshData;
	struct JSkinnedMeshData;

	class JMeshInterface : public JResourceObject
	{ 
	private:
		friend class JModel;
		friend class JResourceManager;
	protected:
		JMeshInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	private:
		virtual bool StuffStaticMesh(JStaticMeshData& meshData, const DirectX::BoundingBox& boundingBox, const DirectX::BoundingSphere& boundingSphere) = 0;
		virtual bool StuffSkinnedMesh(JSkinnedMeshData& meshData, const DirectX::BoundingBox& boundingBox, const DirectX::BoundingSphere& boundingSphere) = 0;
	};
}
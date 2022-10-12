#pragma once
#include"JMeshGeometry.h"

namespace JinEngine
{
	class JStaticMeshGeometry final : public JMeshGeometry
	{
		REGISTER_CLASS(JStaticMeshGeometry)
	public: 
		J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept final;
	private:
		bool WriteMeshData(JMeshGroup& meshGroup)final;
		bool ReadMeshData()final;
		bool ImportMesh(JMeshGroup& meshGroup)final;
	private:
		static void RegisterJFunc();
	private: 
		JStaticMeshGeometry(const JMeshInitData& initdata);
		~JStaticMeshGeometry();
	};
}
 
#pragma once
#include"JMeshGeometry.h"

namespace JinEngine
{
	struct JSkeleton;
	class JSkeletonAsset;
	class JSkinnedMeshGeometry final : public JMeshGeometry
	{
		REGISTER_CLASS(JSkinnedMeshGeometry)
	private:    
		JSkeletonAsset* skeletonAsset = nullptr;
	public:
		JSkeletonAsset* GetSkeletonAsset()const noexcept; 
		J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept final;
	private: 
		void SetSkeletonAsset(JSkeletonAsset* skeletonAsset)noexcept;
	private:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		bool WriteMeshData(JMeshGroup& meshGroup)final;
		bool ReadMeshData()final;
		Core::JOwnerPtr<JSkeleton> ReadSkeletonData();
		bool ImportMesh(JMeshGroup& meshGroup)final;;
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		static void RegisterJFunc();
	private:
		JSkinnedMeshGeometry(const JMeshInitData& initdata);
		~JSkinnedMeshGeometry();
	};
}
#pragma once
#include"JMeshGeometry.h"

namespace JinEngine
{ 
	class JSkeletonAsset;
	class JSkinnedMeshGeometryPrivate;
	class JSkinnedMeshGeometry final : public JMeshGeometry
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JSkinnedMeshGeometry)
	public: 
		class InitData : public JMeshGeometry::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
		private:
			friend class JSkinnedMeshGeometry;
		private:
			Core::JUserPtr<JSkeletonAsset> skeletonAsset;
		public:
			InitData(const uint8 formatIndex, JDirectory* directory, std::unique_ptr<JMeshGroup>&& skinnedMeshGroup);
			InitData(const size_t guid,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& skinnedMeshGroup);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& skinnedMeshGroup);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadMetaData final : public JMeshGeometry::LoadMetaData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			J_MESHGEOMETRY_TYPE meshType;
		public:
			LoadMetaData(JDirectory* directory);
		};
	private:
		friend class JSkinnedMeshGeometryPrivate;
		class JSkinnedMeshGeometryImpl;
	private:
		std::unique_ptr<JSkinnedMeshGeometryImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept final; 
		JSkeletonAsset* GetSkeletonAsset()const noexcept;
	private:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;   
	private:
		JSkinnedMeshGeometry(InitData& initData);
		~JSkinnedMeshGeometry();
	};
}
 
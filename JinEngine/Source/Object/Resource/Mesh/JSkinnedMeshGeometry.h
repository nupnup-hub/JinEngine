#pragma once
#include"JMeshGeometry.h"

namespace JinEngine
{ 
	class JSkeletonAsset;
	class JSkinnedMeshGeometryPrivate;
	class JSkinnedMeshGeometry final : public JMeshGeometry
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JSkinnedMeshGeometry)
		REGISTER_CLASS_IDENTIFIER_DEFAULT_LAZY_DESTRUCTION
	public: 
		class InitData : public JMeshGeometry::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
		private:
			friend class JSkinnedMeshGeometry;
		private:
			JUserPtr<JSkeletonAsset> skeletonAsset;
		public:
			InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory, std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadMetaData final : public JMeshGeometry::LoadMetaData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			Core::J_MESHGEOMETRY_TYPE meshType;
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JSkinnedMeshGeometryPrivate;
		class JSkinnedMeshGeometryImpl;
	private:
		std::unique_ptr<JSkinnedMeshGeometryImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		Core::J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept final; 
		JUserPtr<JSkeletonAsset> GetSkeletonAsset()const noexcept;
	private:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;   
	private:
		JSkinnedMeshGeometry(InitData& initData);
		~JSkinnedMeshGeometry();
	};
}
 
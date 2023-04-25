#pragma once
#include"JMeshGeometry.h"

namespace JinEngine
{
	class JStaticMeshGeometryPrivate;
	class JStaticMeshGeometry final : public JMeshGeometry
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JStaticMeshGeometry)
	public: 
		class InitData : public JMeshGeometry::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
		public:
			InitData(const uint8 formatIndex, JDirectory* directory, std::unique_ptr<JMeshGroup>&& meshGroup);
			InitData(const size_t guid,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& meshGroup);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& meshGroup);
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
		friend class JStaticMeshGeometryPrivate;
		class JStaticMeshGeometryImpl;
	private:
		std::unique_ptr<JStaticMeshGeometryImpl> impl;
	public: 
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept final;
	private: 
		JStaticMeshGeometry(InitData& initData);
		~JStaticMeshGeometry();
	};
}
 
#pragma once
#include"../JResourceObjectPrivate.h" 
#include"../JResourceObjectEventType.h"

struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace JinEngine
{	
	struct JMeshGroup;
	class JMeshGeometry;  

	class JMeshGeometryPrivate : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			friend class JMeshGeometry;
		private:
			virtual std::unique_ptr<JMeshGroup> ReadMeshGroupData(const std::wstring& path) = 0; 
		};
		class CreateInstanceInterface : public JResourceObjectPrivate::CreateInstanceInterface
		{
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		private: 
			void TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept final;
		};
		class DestroyInstanceInterface : public JResourceObjectPrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced) override;
		};
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
	};
}
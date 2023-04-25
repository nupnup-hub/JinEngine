#pragma once
#include"../JResourceObjectPrivate.h" 

struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace JinEngine
{	
	struct JMeshGroup;
	class JMeshGeometry;

	namespace Graphic
	{
		class JGraphic;
		class JOutline;
	}
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
		private: 
			void TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept final;
		};
		class BufferViewInterface
		{
		private:
			friend class Graphic::JGraphic;
			friend class Graphic::JOutline;
		private:
			static D3D12_VERTEX_BUFFER_VIEW VertexBufferView(JMeshGeometry* mesh)noexcept;
			static D3D12_INDEX_BUFFER_VIEW IndexBufferView(JMeshGeometry* mesh)noexcept;
		};
	};
}
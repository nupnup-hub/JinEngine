/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../JResourceObject.h" 
#include"../JResourceObjectImporter.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../../Core/Geometry/Mesh/JMeshType.h" 
#include<DirectXCollision.h> 

namespace JinEngine
{
	namespace Core
	{
		struct JMeshGroup;
	}
	class JMeshGeometryPrivate;
	class JMaterial;
	 

	class JMeshGeometryImportDesc : public JResourceObjectImportDesc
	{
		REGISTER_CLASS_ONLY_USE_TYPEINFO(JMeshGeometryImportDesc)
	public:
		bool useSplitMesh = false;		//ignore split mesh if skinned geometry
	public:
		JMeshGeometryImportDesc(const Core::JFileImportPathData& importPathData);
	};

	class JMeshGeometry : public JResourceObject,
		public Graphic::JGraphicResourceUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JMeshGeometry) 
	public: 
		class InitData : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::unique_ptr<Core::JMeshGroup> meshGroup;
		public:
			InitData(const Core::JTypeInfo& type,
				const uint8 formatIndex, 
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& meshGroup);
			InitData(const Core::JTypeInfo& type, 
				const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& meshGroup);
			InitData(const Core::JTypeInfo& type, 
				const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& meshGroup);
		public:
			bool IsValidData()const noexcept override;
		};
	protected: 
		class LoadMetaData : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			Core::J_MESHGEOMETRY_TYPE meshType;
		public:
			LoadMetaData(const Core::JTypeInfo& type, const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JMeshGeometryPrivate;
		class JMeshGeometryImpl;
	private:
		std::unique_ptr<JMeshGeometryImpl> impl;
	public:  
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MESH;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
		virtual Core::J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept = 0;
		uint GetTotalVertexCount()const noexcept;
		uint GetTotalIndexCount()const noexcept;
		uint GetTotalSubmeshCount()const noexcept;
		uint GetSubmeshVertexCount(const uint index)const noexcept;
		uint GetSubmeshIndexCount(const uint index)const noexcept;
		uint GetSubmeshBaseVertexLocation(const uint index)const noexcept;
		uint GetSubmeshStartIndexLocation(const uint index)const noexcept;
		uint GetVertexByteSize()const noexcept;
		uint GetVertexBufferByteSize()const noexcept;
		uint GetIndexByteSize()const noexcept;
		uint GetIndexBufferByteSize()const noexcept;
		std::wstring GetSubMeshName(const uint index)const noexcept;
		JUserPtr<JMaterial> GetSubmeshMaterial(const uint index)const noexcept;
		DirectX::BoundingBox GetBoundingBox()const noexcept;
		JVector3<float> GetBoundingBoxCenter()const noexcept;
		JVector3<float> GetBoundingBoxExtent()const noexcept;
		DirectX::BoundingSphere GetBoundingSphere()const noexcept;
		JVector3<float> GetBoundingSphereCenter()const noexcept;
		float GetBoundingSphereRadius()const noexcept; 
	protected:
		Core::JMeshGroup* GetMeshGroupData()const noexcept;
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override; 
	protected:
		JMeshGeometry(InitData& initData);
		~JMeshGeometry();
	};
}
  
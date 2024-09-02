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


#include"JDx12GpuAcceleratorManager.h"
#include"JDx12GpuAcceleratorHolder.h" 
#include"../JGpuAcceleratorInfo.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h" 
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JDx12Utility.h" 
#include"../../GraphicResource/JGraphicResourceInterface.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../Raytracing/Dx/JDx12RaytracingConstants.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../../../Object/Component/Transform/JTransform.h"
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Core/Geometry/Mesh/JMeshStruct.h"
#include<set> 

using namespace DirectX;
namespace JinEngine::Graphic
{
	static constexpr uint defaultHitGroupIndex = 0;
	static constexpr uint fixedElementCount = 8;

	using GeometryDesc = D3D12_RAYTRACING_GEOMETRY_DESC;
	using InstanceDesc = D3D12_RAYTRACING_INSTANCE_DESC;
	using AcceleratorBuildDesc = D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC;

	using GeometryDescVec = std::vector<GeometryDesc>;
	using BlasDescVec = std::vector<AcceleratorBuildDesc>;
	using BlasHolderVec = std::vector<std::unique_ptr<JBlasHolder>>;
	using BlasHolderMap = std::unordered_map <size_t, JBlasHolder*>;
	using InstanceDescVec = std::vector<InstanceDesc>;
	using ComponentCacheVec = std::vector<std::unique_ptr<JComponentInstanceCache>>;
	using ComponentCacheMap = std::unordered_map<size_t, JComponentInstanceCache*>;

	struct AccelerationStructureBuffers
	{
	public:
		JDx12GraphicBuffer accelerationStructure;
		JDx12GraphicBuffer scratch;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo;
	};
	struct ObjectData
	{
	public:
		JUserPtr<JComponent> instanceComp;		//has shape (renderItem, localLight ...)
		JUserPtr<JMeshGeometry> mesh;
	public:
		J_COMPONENT_TYPE compType;
		Core::JEnum detailType;
	public:
		ObjectData() {}
		ObjectData(const JUserPtr<JComponent>& comp)
		{
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_LIGHT)
				Set(Core::ConnectChildUserPtr<JLight>(comp));
			else
				Set(Core::ConnectChildUserPtr<JRenderItem>(comp));
		}
		ObjectData(const JUserPtr<JRenderItem>& rItem)
		{
			Set(rItem);
		}
		ObjectData(const JUserPtr<JLight>& localLight)
		{
			Set(localLight);
		}
	public:
		XMMATRIX GetTransformMatrix()const noexcept
		{
			if (compType == J_COMPONENT_TYPE::ENGINE_LIGHT)
				return XMMatrixTranspose(static_cast<JLight*>(instanceComp.Get())->GetMeshWorldM(true));
			else
				return XMMatrixTranspose(instanceComp->GetOwner()->GetTransform()->GetWorldMatrix().LoadXM());
		}
		uint GetInstanceID(const uint subMeshIndex)const noexcept
		{
			return GetInstanceID(instanceComp.Get(), subMeshIndex);
		}
		static uint GetInstanceID(JComponent* comp, const uint subMeshIndex)noexcept
		{
			auto fUser = comp->ModuleManagedData()->GetFrameUpdateUserInterface();
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_RENDERITEM)
				return fUser->GetFrameIndex(J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT_REF_INFO) + subMeshIndex;
			else
			{
				//light shape는 submesh = 1이여야 한다.
				JLight* lit = static_cast<JLight*>(comp);
				J_LIGHT_TYPE type = lit->GetLightType();
				uint offset = fUser->GetFrameIndex(JLightType::LitToFrameR(type));
				if (type == J_LIGHT_TYPE::POINT)
					return offset + Constants::pointLightAsInstanceIdOffset;
				else if (type == J_LIGHT_TYPE::SPOT)
					return offset + Constants::spotLightAsInstanceIdOffset;
				else if (type == J_LIGHT_TYPE::RECT)
					return offset + Constants::rectLightAsInstanceIdOffset;
				else
					return invalidIndex;
			}
		}
		uint GetInstanceMask()const noexcept
		{
			if (compType == J_COMPONENT_TYPE::ENGINE_LIGHT)
			{
				if (detailType == (uint)J_LIGHT_TYPE::POINT)
					return Constants::pointLightMask;
				else if (detailType == (uint)J_LIGHT_TYPE::SPOT)
					return Constants::spotLightMask;
				if (detailType == (uint)J_LIGHT_TYPE::RECT)
					return Constants::rectLightMask;
				else
					return invalidIndex;
			}
			else
				return Constants::renderItemMask;
		}
		size_t GetGuid()const noexcept
		{
			return instanceComp->GetGuid();
		}
	private:
		XMMATRIX GetOffsetMatrix()const noexcept
		{
			return XMMATRIX();
		}
	public:
		void Set(const JUserPtr<JRenderItem>& rItem)
		{
			compType = J_COMPONENT_TYPE::ENGINE_RENDERITEM;
			instanceComp = rItem;
			mesh = rItem->GetOwner()->GetRenderItem()->GetMesh();
		}
		void Set(const JUserPtr<JLight>& localLight)
		{
			compType = J_COMPONENT_TYPE::ENGINE_LIGHT;
			instanceComp = localLight;

			detailType = (uint)localLight->GetLightType();
			if (detailType == (uint)J_LIGHT_TYPE::POINT)
				mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::SPHERE);
			else if (detailType == (uint)J_LIGHT_TYPE::SPOT)
				mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_CONE);
			else if (detailType == (uint)J_LIGHT_TYPE::RECT)
				mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_HEMI_SPHERE);
		}
	public:
		bool IsValid()
		{
			//Test
			//Skinned buf fix후 수정필요
			//return mesh != nullptr && mesh->GetMeshGeometryType() != Core::J_MESHGEOMETRY_TYPE::SKINNED;
			return mesh != nullptr;
		}
	};
	struct JDx12GpuAcceleratorManager::BuildData
	{
	public:
		JDx12GraphicDevice* device;
		JDx12GraphicResourceManager* gm;
		ID3D12Device5* raytracingDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> raytracingCommandList;
	public:
		std::vector<AccelerationStructureBuffers> intermediateBuffer;
		std::vector<GeometryDesc> geometryDescs;
	public:
		J_GPU_ACCELERATOR_BUILD_OPTION optionFlag;
	public:
		std::vector<ObjectData> objectData;
		std::set<size_t> meshSet;
		uint instanceCount = 0;
		uint newInstanceCount = 0;
		uint blasHolderCount = 0;
	public:
		bool isStatic = false;
		bool isOpaque = false;
		bool allowBuildLightShape = false;
		bool allowReBuild = false;  
	public:
		BuildData(JGraphicDevice* newDevice,
			JGraphicResourceManager* newGm,
			const J_GPU_ACCELERATOR_BUILD_OPTION optionFlag)
			:device(static_cast<JDx12GraphicDevice*>(newDevice)),
			gm(static_cast<JDx12GraphicResourceManager*>(newGm)),
			optionFlag(optionFlag)
		{ 
			raytracingDevice = device->GetRaytracingDevice();
			device->GetPublicCmdList()->QueryInterface(IID_PPV_ARGS(&raytracingCommandList));
			isStatic = Core::HasSQValueEnum(optionFlag, J_GPU_ACCELERATOR_BUILD_OPTION_STATIC);
			isOpaque = Core::HasSQValueEnum(optionFlag, J_GPU_ACCELERATOR_BUILD_OPTION_OPAQUE);
			allowBuildLightShape = Core::HasSQValueEnum(optionFlag, J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE);
			allowReBuild = !isStatic;
		}
	public:
		void PushObjectData(const JGpuAcceleratorBuildDesc& desc)
		{
			const uint objCount = (uint)desc.obj.size();
			const uint litCount = (uint)desc.localLight.size();

			uint totalCount = objCount;
			if (allowBuildLightShape)
				totalCount += litCount;

			//objectData.resize(totalCount);
			for (uint i = 0; i < objCount; ++i)
				PushObjectData(ObjectData(desc.obj[i]->GetRenderItem()));;
			//PushObjectData(ObjectData(desc.obj[i]->GetRenderItem()), i);
			if (allowBuildLightShape)
			{
				for (uint i = 0; i < litCount; ++i)
					PushObjectData(ObjectData(desc.localLight[i]->GetComponent<JLight>()));
				//PushObjectData(ObjectData(desc.localLight[i]->GetComponent<JLight>()), i + objCount);
			}
		}
		void PushObjectData(const JUserPtr<JComponent>& comp)
		{
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_LIGHT)
			{
				if (allowBuildLightShape)
					PushObjectData(ObjectData(Core::ConnectChildUserPtr<JLight>(comp)));
			}
			else
				PushObjectData(ObjectData(Core::ConnectChildUserPtr<JRenderItem>(comp)));
		}
	private:
		void PushObjectData(ObjectData&& data)
		{
			if (!data.IsValid())
				return;

			const uint totalSubmesh = data.mesh->GetTotalSubmeshCount();
			if (meshSet.find(data.mesh->GetGuid()) == meshSet.end())
			{
				meshSet.emplace(data.mesh->GetGuid());
				newInstanceCount += totalSubmesh;
				blasHolderCount += 1;
			}
			instanceCount += totalSubmesh;
			objectData.push_back(std::move(data));
		}
		void PushObjectData(ObjectData&& data, const uint index)
		{
			if (!data.IsValid())
				return;

			const uint totalSubmesh = data.mesh->GetTotalSubmeshCount();
			if (meshSet.find(data.mesh->GetGuid()) == meshSet.end())
			{
				meshSet.emplace(data.mesh->GetGuid());
				newInstanceCount += totalSubmesh;
				blasHolderCount += 1;
			}
			instanceCount += totalSubmesh;
			objectData[index] = std::move(data);
		}
	public:
		bool IsValid()const noexcept
		{
			return raytracingDevice != nullptr && raytracingCommandList != nullptr && objectData.size() > 0;
		}
	};

	using BuildData = JDx12GpuAcceleratorManager::BuildData;

	static size_t CalBufferCapacity(const uint count)
	{
		size_t shift = 2;
		size_t capa = fixedElementCount;
		while (capa < count)
			capa = 1 << (++shift);
		return capa;
	}
	static void BuildGeometryDescsForBottomLevelAS(const BuildData& buildData, const uint dataIndex, const uint submeshIndex, GeometryDesc& desc)
	{
		const auto flag = buildData.isOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		const JUserPtr<JMeshGeometry>& mesh = buildData.objectData[dataIndex].mesh;

		auto gInterface = mesh->ModuleManagedData()->GetGraphicResourceUserInterface();
		auto vertexHolder = buildData.gm->GetDxHolder(J_GRAPHIC_RESOURCE_TYPE::VERTEX, gInterface->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::VERTEX, 0));
		auto indexHolder = buildData.gm->GetDxHolder(J_GRAPHIC_RESOURCE_TYPE::INDEX, gInterface->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::INDEX, 0));

		const bool useStaticMeshByte = mesh->GetMeshGeometryType() == Core::J_MESHGEOMETRY_TYPE::STATIC;
		//|| mesh->GetMeshGeometryType() == Core::J_MESHGEOMETRY_TYPE::SKINNED
		desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		desc.Flags = flag;
		desc.Triangles.VertexBuffer.StartAddress = vertexHolder->GetResource()->GetGPUVirtualAddress() + mesh->GetVertexByteSize() * mesh->GetSubmeshBaseVertexLocation(submeshIndex);
		//desc.Triangles.VertexBuffer.StrideInBytes = mesh->GetVertexByteSize();
		desc.Triangles.VertexBuffer.StrideInBytes = useStaticMeshByte ? sizeof(Core::JStaticMeshVertex) : mesh->GetVertexByteSize();
		desc.Triangles.VertexCount = mesh->GetSubmeshVertexCount(submeshIndex);
		desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.Triangles.IndexBuffer = indexHolder->GetResource()->GetGPUVirtualAddress() + mesh->GetIndexByteSize() * mesh->GetSubmeshStartIndexLocation(submeshIndex);
		desc.Triangles.IndexCount = mesh->GetSubmeshIndexCount(submeshIndex);
		desc.Triangles.IndexFormat = mesh->GetIndexByteSize() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		desc.Triangles.Transform3x4 = NULL; 
	}
	static void BuildInstanceDesc(const BuildData& buildData,
		const JDx12GraphicBuffer& blasBuffer,
		const ObjectData& data,
		const uint subMeshIndex,
		const uint hitGroupShaderIndex,
		InstanceDesc& desc)
	{
		JMatrix4x4 matrix;
		matrix.StoreXM(data.GetTransformMatrix());

		memcpy(desc.Transform, matrix.m, sizeof(float) * 12);
		desc.AccelerationStructure = blasBuffer.GetResource()->GetGPUVirtualAddress();
		desc.Flags = 0;
		desc.InstanceID = data.GetInstanceID(subMeshIndex);
		desc.InstanceMask = data.GetInstanceMask();
		desc.InstanceContributionToHitGroupIndex = hitGroupShaderIndex;

		matrix.StoreXM(data.GetTransformMatrix());
	}
	static void BuildInstanceDesc(const BuildData& buildData,
		const JDx12GraphicBuffer& blasBuffer,
		const uint objIndex,
		const uint subMeshIndex,
		const uint hitGroupShaderIndex,
		InstanceDesc& desc)
	{
		BuildInstanceDesc(buildData, blasBuffer, buildData.objectData[objIndex], subMeshIndex, hitGroupShaderIndex, desc);
	}
	static void UpdateInstanceID(const BuildData& buildData,
		_Inout_ InstanceDescVec& instanceDesc,
		_Inout_ ComponentCacheVec& compCacheVec, 
		const uint cacheStIndex,
		const uint removedInstanceDescCount,
		const uint removedObjectCount)
	{ 
		for (uint i = cacheStIndex; i < compCacheVec.size(); ++i)
		{
			compCacheVec[i]->descStIndex -= removedInstanceDescCount;
			compCacheVec[i]->index -= removedObjectCount;

			ObjectData data(compCacheVec[i]->comp);
			const uint subMeshCount = data.mesh->GetTotalSubmeshCount();

			const uint descIndex = compCacheVec[i]->descStIndex;
			for (uint j = 0; j < subMeshCount; ++j)
				instanceDesc[descIndex + j].InstanceID = data.GetInstanceID(j);
		}
	}
	static void BuildBottomLevelAS(BuildData& buildData, const GeometryDesc& geometryDescs, AcceleratorBuildDesc& blasDesc, AccelerationStructureBuffers& buffers)
	{
		static constexpr auto staticFlag = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
		static constexpr auto dynamicFlag = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		const auto flag = buildData.isStatic ? staticFlag : dynamicFlag;

		auto& bottomLevelInputs = blasDesc.Inputs;
		bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		bottomLevelInputs.Flags = flag;
		bottomLevelInputs.NumDescs = 1;		//geometry count
		bottomLevelInputs.pGeometryDescs = &geometryDescs;

		auto& bottomLevelPrebuildInfo = buffers.preBuildInfo;
		buildData.raytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
		 
		buffers.scratch = JDx12GraphicBuffer(L"AcceleratiorScratch", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS, bottomLevelPrebuildInfo.ScratchDataSizeInBytes);
		//buffers[i].scratch.SetFixedInitState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		buffers.scratch.Build(buildData.device, 1);

		// Allocate resources for acceleration structures.
		// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
		// Default heap is OK since the application doesn뭪 need CPU read/write access to them. 
		// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
		// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
		//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
		//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.

		buffers.accelerationStructure = JDx12GraphicBuffer(L"Acceleratior", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes);
		buffers.accelerationStructure.SetFixedInitState(D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		buffers.accelerationStructure.Build(buildData.device, 1);

		// bottom-level AS desc.			
		blasDesc.ScratchAccelerationStructureData = buffers.scratch.GetResource()->GetGPUVirtualAddress();
		blasDesc.DestAccelerationStructureData = buffers.accelerationStructure.GetResource()->GetGPUVirtualAddress();
		blasDesc.SourceAccelerationStructureData = NULL;
	}
	static void BuildBottomLevelAS(BuildData& buildData,
		_Inout_ BlasDescVec& blasDesc,
		_Inout_ BlasHolderVec& blasVec,
		_Inout_ BlasHolderMap& blasMap,
		_Inout_ InstanceDescVec& instanceDesc,
		_Inout_ ComponentCacheVec& compCacheVec,
		_Inout_ ComponentCacheMap& compCacheMap)
	{
		auto& geometryDescs = buildData.geometryDescs;
		auto& intermediateBuffer = buildData.intermediateBuffer;

		geometryDescs.resize(buildData.newInstanceCount);
		intermediateBuffer.resize(buildData.newInstanceCount);
		blasDesc.resize(buildData.newInstanceCount);

		const uint existBlasHolder = (uint)blasVec.size();
		blasVec.resize(existBlasHolder + buildData.blasHolderCount);
		blasMap.reserve(existBlasHolder + buildData.blasHolderCount);

		const uint existInstanceDesc = (uint)instanceDesc.size();
		const uint existInstanceCache = (uint)compCacheVec.size();
		instanceDesc.resize(existInstanceDesc + buildData.instanceCount);
		compCacheVec.resize(existInstanceCache + buildData.objectData.size());
		compCacheMap.reserve(existInstanceCache + buildData.objectData.size());

		int blasIndex = 0;
		int blasHolderIndex = existBlasHolder;
		int intermediateIndex = existBlasHolder;
		int instanceDescIndex = existInstanceDesc;
		int instanceCompIndex = existInstanceCache;

		const uint objDataCount = (uint)buildData.objectData.size();
		for (uint i = 0; i < objDataCount; ++i)
		{
			auto& objData = buildData.objectData[i];
			const uint subMeshCount = objData.mesh->GetTotalSubmeshCount();

			JBlasHolder* existHolder = nullptr;
			auto existData = blasMap.find(objData.mesh->GetGuid());
			if (existData == blasMap.end())
			{
				blasVec[blasHolderIndex] = std::make_unique<JBlasHolder>(blasHolderIndex);
				JBlasHolder* newHolder = blasVec[blasHolderIndex].get();
				newHolder->refInstanceCount = 1;
				newHolder->buffer.resize(subMeshCount);

				for (uint j = 0; j < subMeshCount; ++j)
				{ 
					BuildGeometryDescsForBottomLevelAS(buildData, i, j, geometryDescs[blasIndex]);
					BuildBottomLevelAS(buildData, geometryDescs[blasIndex], blasDesc[blasIndex], intermediateBuffer[blasIndex]);
					newHolder->buffer[j] = std::move(intermediateBuffer[blasIndex].accelerationStructure);
					++blasIndex;
				}
				existHolder = newHolder;
				blasMap.emplace(objData.mesh->GetGuid(), newHolder);
				++blasHolderIndex;
			}
			else
			{
				existHolder = existData->second;
				++existHolder->refInstanceCount;
			}

			auto compCache = std::make_unique<JComponentInstanceCache>(objData.instanceComp, instanceCompIndex, instanceDescIndex);
			compCacheMap.emplace(objData.instanceComp->GetGuid(), compCache.get());
			compCacheVec[instanceCompIndex] = std::move(compCache);
			++instanceCompIndex;

			for (uint j = 0; j < subMeshCount; ++j)
			{
				BuildInstanceDesc(buildData, existHolder->buffer[j], i, j, defaultHitGroupIndex, instanceDesc[instanceDescIndex]);
				++instanceDescIndex;
			}
		}
	}
	static void BuildTopLevelAS(const BuildData& buildData, const uint instanceCount, AcceleratorBuildDesc& desc, AccelerationStructureBuffers& buffers)
	{
		// Get required sizes for an acceleration structure.
		auto& topLevelBuildDesc = desc;
		auto& topLevelInputs = topLevelBuildDesc.Inputs;
		topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		topLevelInputs.NumDescs = instanceCount;		//instance count
		topLevelInputs.pGeometryDescs = nullptr;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& topLevelPrebuildInfo = buffers.preBuildInfo;
		buildData.raytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

		buffers.scratch = JDx12GraphicBuffer(L"AcceleratiorScratch", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS, topLevelPrebuildInfo.ScratchDataSizeInBytes);
		//buffers.scratch.SetFixedInitState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		buffers.scratch.Build(buildData.device, 1);

		// Allocate resources for acceleration structures.
		// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
		// Default heap is OK since the application doesn뭪 need CPU read/write access to them. 
		// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
		// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
		//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
		//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.

		buffers.accelerationStructure = JDx12GraphicBuffer(L"Acceleratior", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);
		buffers.accelerationStructure.SetFixedInitState(D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		buffers.accelerationStructure.Build(buildData.device, 1);

		topLevelBuildDesc.ScratchAccelerationStructureData = buffers.scratch.GetResource()->GetGPUVirtualAddress();
		topLevelBuildDesc.DestAccelerationStructureData = buffers.accelerationStructure.GetResource()->GetGPUVirtualAddress();
		topLevelBuildDesc.SourceAccelerationStructureData = NULL;
	}
	static JDx12GraphicBufferT<InstanceDesc> BuildInstanceBuffer(const BuildData& buildData, const std::vector<InstanceDesc>& instanceDesc)
	{
		JDx12GraphicBufferT<InstanceDesc> buffer(L"AsInstance", J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		buffer.Build(buildData.device, CalBufferCapacity((uint)instanceDesc.size()));
		buffer.CopyData(0, (int)instanceDesc.size(), instanceDesc.data());
		return std::move(buffer);
	}

	static void SubmitBuildBottomLevelASCommand(const BuildData& buildData, const AcceleratorBuildDesc& desc)
	{
		//리소스는 NULL일 수 있으며, 이는 모든 UAV 액세스에 장벽이 필요할 수 있음을 나타냅니다.
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
		buildData.raytracingCommandList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
		buildData.raytracingCommandList->ResourceBarrier(1, &barrier);
	}
	static void SubmitBuildBottomLevelASCommand(const BuildData& buildData, const std::vector<AcceleratorBuildDesc>& desc)
	{
		//리소스는 NULL일 수 있으며, 이는 모든 UAV 액세스에 장벽이 필요할 수 있음을 나타냅니다.
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
		for (uint i = 0; i < (uint)desc.size(); ++i)
			buildData.raytracingCommandList->BuildRaytracingAccelerationStructure(&desc[i], 0, nullptr);
		buildData.raytracingCommandList->ResourceBarrier(1, &barrier);
	}
	static void SubmitBuildTopLevelASCommand(const BuildData& buildData, const AccelerationStructureBuffers& buffers, const AcceleratorBuildDesc& desc)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(buffers.accelerationStructure.GetResource());
		buildData.raytracingCommandList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
		buildData.raytracingCommandList->ResourceBarrier(1, &barrier);
	}

	JDx12GpuAcceleratorManager::~JDx12GpuAcceleratorManager()
	{
		ClearResource();
	}
	void JDx12GpuAcceleratorManager::Initialize(JGraphicDevice* device)
	{
		if (!IsSameDevice(device))
			return;

		JGpuAcceleratorManager::Initialize(device);
		BuildResource(device);
	}
	void JDx12GpuAcceleratorManager::Clear()
	{
		ClearResource();
		JGpuAcceleratorManager::Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GpuAcceleratorManager::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JGpuAcceleratorInfo* JDx12GpuAcceleratorManager::GetInfo(const uint index)const noexcept
	{
		return index < infoVec.size() ? infoVec[index].Get() : nullptr;
	}
	JDx12GpuAcceleratorHolder* JDx12GpuAcceleratorManager::GetDx12Holder(JGpuAcceleratorInfo* info)const noexcept
	{
		return static_cast<JDx12GpuAcceleratorHolder*>(JGpuAcceleratorManager::GetHolder(info));
	}
	JUserPtr<JGpuAcceleratorInfo> JDx12GpuAcceleratorManager::Create(JGraphicDevice* device, JGraphicResourceManager* gm, const JGpuAcceleratorBuildDesc& desc)
	{
		BuildData buildData(device, gm, desc.flag);
		buildData.PushObjectData(desc);

		std::unique_ptr<JDx12GpuAcceleratorHolder> holder = BuildAcceleratorStructure(buildData);
		if (holder == nullptr)
			return nullptr;

		JOwnerPtr<JDx12GpuAcceleratorInfo> owner = CreateInfo(std::move(holder), desc);
		JUserPtr<JDx12GpuAcceleratorInfo> user = owner;
		user->SetArrayIndex((uint)infoVec.size());

		infoVec.push_back(std::move(owner));
		return user;
	}
	bool JDx12GpuAcceleratorManager::Destroy(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info)
	{
		if (info == nullptr)
			return false;

		const int arrayIndex = info->GetArrayIndex();
		const int vecCount = (int)infoVec.size();
		for (int i = arrayIndex + 1; i < vecCount; ++i)
			infoVec[i]->SetArrayIndex(infoVec[i]->GetArrayIndex() - 1);
		infoVec.erase(infoVec.begin() + arrayIndex);
		return false;
	}
	void JDx12GpuAcceleratorManager::UpdateTransform(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
	{
		if (info == nullptr)
			return;

		BuildData buildData(device, gm, info->GetBuildOption());
		buildData.PushObjectData(comp);
		if (!buildData.allowReBuild)
			return;

		UpdateInstance(buildData, static_cast<JDx12GpuAcceleratorHolder*>(GetHolder(info)), info->GetBuildOption());
	}
	void JDx12GpuAcceleratorManager::Add(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
	{
		if (info == nullptr)
			return;

		BuildData buildData(device, gm, info->GetBuildOption());
		buildData.PushObjectData(comp);

		if (!buildData.allowReBuild)
			return;

		AddBottomLevelAs(buildData, static_cast<JDx12GpuAcceleratorHolder*>(GetHolder(info)));
	}
	void JDx12GpuAcceleratorManager::Remove(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
	{
		if (info == nullptr)
			return;

		BuildData buildData(device, gm, info->GetBuildOption());
		buildData.PushObjectData(comp);

		if (!buildData.allowReBuild)
			return;

		RemoveBottomLevelAs(buildData, static_cast<JDx12GpuAcceleratorHolder*>(GetHolder(info)));
	}
	JOwnerPtr<JDx12GpuAcceleratorInfo> JDx12GpuAcceleratorManager::CreateInfo(std::unique_ptr<JGpuAcceleratorHolder>&& holder, const JGpuAcceleratorBuildDesc& desc)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JDx12GpuAcceleratorInfo>(desc.flag, std::move(holder), this);
	}
	std::unique_ptr<JDx12GpuAcceleratorHolder> JDx12GpuAcceleratorManager::BuildAcceleratorStructure(BuildData& buildData)
	{
		if (!buildData.IsValid())
			return nullptr;

		bool startCommandThisCreation = false;
		buildData.device->StartPublicCommandSet(startCommandThisCreation);

		BlasDescVec blasDesc;
		BlasHolderVec blasVec;
		BlasHolderMap blasMap;
		InstanceDescVec instanceDesc;
		ComponentCacheVec compCacheVec;
		ComponentCacheMap compCacheMap;
		BuildBottomLevelAS(buildData, blasDesc, blasVec, blasMap, instanceDesc, compCacheVec, compCacheMap);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc;
		AccelerationStructureBuffers tlasBuffers;
		BuildTopLevelAS(buildData, buildData.instanceCount, tlasDesc, tlasBuffers);

		JDx12GraphicBufferT<D3D12_RAYTRACING_INSTANCE_DESC> instanceBuffer = BuildInstanceBuffer(buildData, instanceDesc);

		tlasDesc.Inputs.InstanceDescs = instanceBuffer.GetResource()->GetGPUVirtualAddress();
		tlasDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

		SubmitBuildBottomLevelASCommand(buildData, blasDesc);
		SubmitBuildTopLevelASCommand(buildData, tlasBuffers, tlasDesc);
		buildData.device->EndPublicCommandSet(startCommandThisCreation, true);

		std::unique_ptr<JTlasHolder> tlas = std::make_unique<JTlasHolder>(std::move(tlasBuffers.accelerationStructure));
		return std::make_unique<JDx12GpuAcceleratorHolder>(std::move(tlas),
			std::move(blasVec),
			std::move(blasMap),
			std::move(instanceBuffer),
			std::move(instanceDesc),
			std::move(compCacheVec),
			std::move(compCacheMap));
	}
	void JDx12GpuAcceleratorManager::UpdateInstance(const BuildData& buildData, JDx12GpuAcceleratorHolder* holder, const J_GPU_ACCELERATOR_BUILD_OPTION preBuildOption)
	{
		if (!buildData.IsValid())
			return;

		InstanceDescVec& instanceDesc = holder->instanceDescCacheVec;
		ComponentCacheVec& compCacheVec = holder->compCacheVec;
		ComponentCacheMap& compCacheMap = holder->compCacheMap;

		uint updatedStDescIndex = UINT_MAX;
		uint updatedStCompIndex = UINT_MAX;
		uint objCount = (uint)buildData.objectData.size();

		for (uint i = 0; i < objCount; ++i)
		{
			auto& objData = buildData.objectData[i];
			auto blasPtr = holder->GetBlas(objData.mesh->GetGuid());
			if (blasPtr == nullptr)
				continue;

			JComponentInstanceCache* compCache = compCacheMap.find(objData.instanceComp->GetGuid())->second;
			if (updatedStCompIndex > compCache->index)
			{
				updatedStCompIndex = compCache->index;
				updatedStDescIndex = compCache->descStIndex;
			}
		}
		if (updatedStDescIndex == UINT_MAX)
			return;

		bool startCommandThisCreation = false;
		buildData.device->StartPublicCommandSet(startCommandThisCreation);

		const uint existInstanceCount = (uint)holder->instanceDescCacheVec.size();
		const uint existInstanceCacheCount = (uint)holder->compCacheVec.size();
		const uint copyStIndex = updatedStDescIndex;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc;
		AccelerationStructureBuffers tlasBuffers;
		BuildTopLevelAS(buildData, existInstanceCount, tlasDesc, tlasBuffers);

		JDx12GraphicBuffer* instanceData = &holder->instanceData;

		uint updatedCount = 0;
		for (uint i = updatedStCompIndex; i < existInstanceCacheCount; ++i)
		{
			ObjectData data(compCacheVec[i]->comp);
			auto blasPtr = holder->GetBlas(data.mesh->GetGuid());
			const uint subMeshCount = (uint)data.mesh->GetTotalSubmeshCount();
			for (uint j = 0; j < subMeshCount; ++j)
			{
				BuildInstanceDesc(buildData, blasPtr->buffer[j], data, j, defaultHitGroupIndex, instanceDesc[updatedStDescIndex]);
				++updatedStDescIndex;
				++updatedCount;
			}
		}
		instanceData->CopyData(copyStIndex, updatedCount, &instanceDesc[copyStIndex]);

		tlasDesc.Inputs.InstanceDescs = instanceData->GetResource()->GetGPUVirtualAddress();
		tlasDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

		SubmitBuildTopLevelASCommand(buildData, tlasBuffers, tlasDesc);
		buildData.device->EndPublicCommandSet(startCommandThisCreation, true);

		holder->Swap(std::make_unique<JTlasHolder>(std::move(tlasBuffers.accelerationStructure)));
	}
	void JDx12GpuAcceleratorManager::AddBottomLevelAs(BuildData& buildData, JDx12GpuAcceleratorHolder* holder)
	{
		if (!buildData.IsValid())
			return;

		if (!holder->HasData())
			BuildAcceleratorStructure(buildData);
		else
		{
			bool startCommandThisCreation = false;
			buildData.device->StartPublicCommandSet(startCommandThisCreation);

			const uint existBlasCount = (uint)holder->blasVec.size();
			const uint existInstanceCount = (uint)holder->instanceDescCacheVec.size();
			const uint newInstanceCount = (uint)existInstanceCount + buildData.instanceCount;

			BlasDescVec blasDesc;
			BlasHolderVec& blasVec = holder->blasVec;
			BlasHolderMap& blasMap = holder->blasMap;
			InstanceDescVec& instanceDesc = holder->instanceDescCacheVec;
			ComponentCacheVec& compCacheVec = holder->compCacheVec;
			ComponentCacheMap& compCacheMap = holder->compCacheMap;

			BuildBottomLevelAS(buildData, blasDesc, blasVec, blasMap, instanceDesc, compCacheVec, compCacheMap);
			holder->blasVec.resize(holder->blasMap.size());

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc;
			AccelerationStructureBuffers tlasBuffers;
			BuildTopLevelAS(buildData, newInstanceCount, tlasDesc, tlasBuffers);

			JDx12GraphicBuffer* instanceData = &holder->instanceData;
			if (instanceData->GetElementCount() < newInstanceCount)
			{
				instanceData->Build(buildData.device, CalBufferCapacity(newInstanceCount));
				instanceData->CopyData(0, instanceDesc.size(), instanceDesc.data());
			}
			else
				instanceData->CopyData(existInstanceCount, buildData.instanceCount, &instanceDesc[existInstanceCount]);

			tlasDesc.Inputs.InstanceDescs = instanceData->GetResource()->GetGPUVirtualAddress();
			tlasDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

			if (existBlasCount != holder->blasVec.size())
				SubmitBuildBottomLevelASCommand(buildData, blasDesc);
			SubmitBuildTopLevelASCommand(buildData, tlasBuffers, tlasDesc);
			buildData.device->EndPublicCommandSet(startCommandThisCreation, true);

			holder->Swap(std::make_unique<JTlasHolder>(std::move(tlasBuffers.accelerationStructure)));
		}
	}
	void JDx12GpuAcceleratorManager::RemoveBottomLevelAs(const BuildData& buildData, JDx12GpuAcceleratorHolder* holder)
	{
		if (!buildData.IsValid())
			return;

		bool startCommandThisCreation = false;
		buildData.device->StartPublicCommandSet(startCommandThisCreation);
		 
		uint removedObjectCount = (uint)buildData.objectData.size();
		uint removedInstanceCount = buildData.instanceCount;

		InstanceDescVec& instanceDesc = holder->instanceDescCacheVec;
		ComponentCacheVec& compCacheVec = holder->compCacheVec;
		ComponentCacheMap& compCacheMap = holder->compCacheMap;
		uint existDescCount = (uint)instanceDesc.size();

		//remove하는 obj count가 현개 cache된 count보다 같거나 많을 경우 Clear
		//Gpu Acc에 등록되는 경우와 Remove되는 경우의 Object type이 같아야한다.
		if (removedInstanceCount >= compCacheVec.size())
		{
			holder->Clear();
			return;
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc;
		AccelerationStructureBuffers tlasBuffers;
		BuildTopLevelAS(buildData, instanceDesc.size() - removedInstanceCount, tlasDesc, tlasBuffers);
 
		auto& objData = buildData.objectData[0];
		auto blasPtr = holder->GetBlas(objData.mesh->GetGuid());
		if (blasPtr == nullptr)
			return;
		 
		JComponentInstanceCache* compCache = compCacheMap.find(objData.instanceComp->GetGuid())->second;

		const uint updatedStCompIndex = compCache->index;
		const uint updatedStDescIndex = compCache->descStIndex;

		const bool hasNext = uint(compCacheVec.size()) > updatedStCompIndex + 1;
		const uint removedInstanceDescCount = hasNext ? compCacheVec[updatedStCompIndex + 1]->descStIndex - updatedStCompIndex : compCacheVec.size() - updatedStCompIndex;

		instanceDesc.erase(instanceDesc.begin() + compCache->descStIndex, instanceDesc.begin() + compCache->descStIndex + removedInstanceDescCount);
		compCacheMap.erase(objData.instanceComp->GetGuid());
		compCacheVec.erase(compCacheVec.begin() + compCache->index);

		--blasPtr->refInstanceCount;
		if (blasPtr->refInstanceCount == 0)
			holder->RemoveBlas(blasPtr->index, objData.mesh->GetGuid()); 

		//Update desc & cache unique data(ID or Index)
		if (updatedStDescIndex != UINT_MAX)
			UpdateInstanceID(buildData, instanceDesc, compCacheVec, updatedStCompIndex, removedInstanceDescCount, 1);

		JDx12GraphicBuffer* instanceData = &holder->instanceData;
		uint downCapcityBorder = (instanceData->GetElementCount() / 2);
		if (downCapcityBorder > (uint)instanceDesc.size())
		{
			uint newCapacity = downCapcityBorder < fixedElementCount ? fixedElementCount : downCapcityBorder;
			instanceData->Build(buildData.device, newCapacity);
			instanceData->CopyData(0, (uint)instanceDesc.size(), instanceDesc.data());
		}
		else if (instanceDesc.size() - updatedStDescIndex > 0)
			instanceData->CopyData(updatedStDescIndex, instanceDesc.size() - updatedStDescIndex, &instanceDesc[updatedStDescIndex]);

		tlasDesc.Inputs.InstanceDescs = instanceData->GetResource()->GetGPUVirtualAddress();
		tlasDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

		SubmitBuildTopLevelASCommand(buildData, tlasBuffers, tlasDesc);

		holder->Swap(std::make_unique<JTlasHolder>(std::move(tlasBuffers.accelerationStructure)));
		buildData.device->EndPublicCommandSet(startCommandThisCreation, true);
	} 
	void JDx12GpuAcceleratorManager::BuildResource(JGraphicDevice* device)
	{

	}
	void JDx12GpuAcceleratorManager::ClearResource()
	{
		infoVec.clear();
	}
	void JDx12GpuAcceleratorManager::RegisterTypeData()
	{
		using JAllocationDesc = JinEngine::Core::JAllocationDesc;
		using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
		using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
		using ReceiverPtr = JAllocationDesc::ReceiverPtr;
		using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
		using MemIndex = JAllocationDesc::MemIndex;

		NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
		{
			JDx12GpuAcceleratorInfo* movedInfo = static_cast<JDx12GpuAcceleratorInfo*>(movedPtr);
			JDx12GpuAcceleratorManager* manager = movedInfo->manager;

			//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
			//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
			//Release() 한다음 Reset()을 호출해야한다. 
			//2024-08-15 수정 포인터만 변경하는 Swap 사용
			manager->infoVec[movedInfo->GetArrayIndex()].Swap(movedInfo);
		};
		auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
		std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

		desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
		JDx12GpuAcceleratorInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
	}

	JDx12AcceleratorResourceComputeSet::JDx12AcceleratorResourceComputeSet(JDx12GpuAcceleratorManager* am, const JUserPtr<JGpuAcceleratorInfo>& aInfo)
		:am(am), info(aInfo.Get()), holder(info != nullptr ? am->GetDx12Holder(info) : nullptr)
	{}
	JDx12AcceleratorResourceComputeSet::JDx12AcceleratorResourceComputeSet(JDx12GpuAcceleratorManager* am, JGpuAcceleratorInterface* user)
		: am(am), info(am->GetInfo(user->HasInfo() ? user->GetArrayIndex() : invalidIndex)), holder(info != nullptr ? am->GetDx12Holder(info) : nullptr)
	{}
	bool JDx12AcceleratorResourceComputeSet::IsValid()const noexcept
	{
		return info != nullptr;
	}
}
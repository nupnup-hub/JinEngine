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
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../Core/JCoreEssential.h" 
#include <dxc/dxcapi.h>

namespace JinEngine
{
	namespace Graphic
	{
        // Shader record = {{Shader ID}, {RootArguments}}
        class JShaderRecord
        {
        public:
            struct PointerWithSize
            {
            public:
                void* ptr;
                uint size;
            public:
                PointerWithSize();
                PointerWithSize(void* _ptr, uint _size);
            };
        public:
            PointerWithSize shaderIdentifier;
            PointerWithSize localRootArguments;
        public:
            JShaderRecord(void* pShaderIdentifier, uint shaderIdentifierSize);
            JShaderRecord(void* pShaderIdentifier, uint shaderIdentifierSize, void* pLocalRootArguments, uint localRootArgumentsSize);
        public:
            void CopyTo(void* dest) const;
        };
        class JShaderTable : public JDx12GraphicBuffer
        {
        public:
            using ShaderID = void*;
            using LocalRootArg = void*;
            using ShaderIDMap = std::unordered_map<ShaderID, std::wstring>;
        public:
            uint8_t* mappedShaderRecords;
            // Debug support 
            std::vector<JShaderRecord> shaderRecords;
        public:
            JShaderTable(ID3D12Device5* device, uint numShaderRecords, uint shaderRecordSize, const std::wstring& resourceName = L"");
            JShaderTable(const JShaderTable& rhs) = delete;
            JShaderTable& operator=(const JShaderTable& rhs) = delete;
            JShaderTable(JShaderTable&& rhs) = default;
            JShaderTable& operator=(JShaderTable&& rhs) = default;
            ~JShaderTable() = default;
        public:
            void PushBack(const JShaderRecord& shaderRecord);
        public:
            uint GetShaderRecordSize();
            uint GeNumShaderRecords();
        public:
            // Pretty-print the shader records.
            void DebugPrint(ShaderIDMap shaderIdToStringMap);
            // Pretty-print a state object tree.
        private:
            uint Align(uint size, uint alignment);
        };
        class JDx12RaytracingUtility
        {
        public:
            static void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc);
        };
        using JDx12RtUtil = JDx12RaytracingUtility;

        struct JStateObjectBuildData
        {
        public:
            CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
            Microsoft::WRL::ComPtr<IDxcBlob> shader;
        public:
            uint maxRecursion = 1;
        };
        template<uint count>
        struct JShaderTableBuild
        {
        private:
            static constexpr size_t defaultIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        private:
            using ShaderID = JShaderTable::ShaderID;
            using LocalRootArg = JShaderTable::LocalRootArg;
            using ShaderIDMap = JShaderTable::ShaderIDMap;
        private:
            ShaderID shaderID[count];
            std::unique_ptr<JShaderTable> table;
        private:
            size_t shaderIdSize = 0;
            int idIndex = 0;
            int recordIndex = 0;
        public:
            JShaderTableBuild(ID3D12Device5* device, const size_t shaderIdSize = defaultIdSize, const std::wstring& name = L"ShaderTable")
                :shaderIdSize(shaderIdSize), idIndex(0), recordIndex(0)
            {
                table = std::make_unique<JShaderTable>(device, count, shaderIdSize, name);
            }
            JShaderTableBuild(ID3D12Device5* device, const size_t shaderIdSize, const size_t shaderRecordSize, const std::wstring& name = L"ShaderTable")
                :shaderIdSize(shaderIdSize), idIndex(0), recordIndex(0)
            {
                table = std::make_unique<JShaderTable>(device, count, shaderRecordSize, name);
            }
        public:
            void PushID(ID3D12StateObjectProperties* properties, const std::wstring& shaderName)
            {
                shaderID[idIndex] = properties->GetShaderIdentifier(shaderName.c_str());
                ++idIndex;
            }
            void PushID(ID3D12StateObjectProperties* properties, const std::wstring& shaderName, JShaderTable::ShaderIDMap& map)
            {
                PushID(properties, shaderName);
                map.emplace(shaderID[idIndex - 1], shaderName);
            }
            void PushRecord()
            {
                table->PushBack(JShaderRecord(shaderID[recordIndex], shaderIdSize, nullptr, 0));
                ++recordIndex;
            }
            void PushRecord(LocalRootArg localArg, const size_t localRootArgSize)
            {
                table->PushBack(JShaderRecord(shaderID[recordIndex], shaderIdSize, localArg, localRootArgSize));
                ++recordIndex;
            }
            void PushAllRecord()
            {
                for (uint i = 0; i < count; ++i)
                    PushRecord();
            }
        public:
            void DebugPrint(const ShaderIDMap& map)
            {
                table->DebugPrint(map);
            }
        public:
            std::unique_ptr<JShaderTable> Release()
            {
                return std::move(table);
            }
        };
	}
}
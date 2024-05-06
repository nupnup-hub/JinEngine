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
#include"../JResourceObjectPrivate.h" 
#include"../JResourceObjectEventType.h"

struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace JinEngine
{
	namespace Core
	{
		struct JMeshGroup;
	}
	class JMeshGeometry;

	class JMeshGeometryPrivate : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			friend class JMeshGeometry;
		private:
			virtual std::unique_ptr<Core::JMeshGroup> ReadMeshGroupData(const std::wstring& path) = 0;
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
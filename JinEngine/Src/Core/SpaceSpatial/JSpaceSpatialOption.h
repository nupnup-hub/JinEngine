#pragma once
#include"JSpaceSpatialType.h"
#include"../Pointer/JOwnerPtr.h"
#include<fstream>

namespace JinEngine
{
	class JGameObject;
	namespace Core
	{
		class JSpaceSpatialOption
		{
		public:
			JUserPtr<JGameObject> innerRoot = nullptr;
			JUserPtr<JGameObject> debugRoot = nullptr;
		public:
			bool isSpaceSpatialActivated = false;
			bool isDebugActivated = false;
			bool isDebugLeafOnly = true;
			bool isCullingActivated = false;
		public:
			JSpaceSpatialOption() = default;
			JSpaceSpatialOption(JUserPtr<JGameObject> innerRoot,
				JUserPtr<JGameObject> debugRoot,
				bool isSpaceSpatialActivated, 
				bool isDebugActivated, 
				bool isDebugLeafOnly,
				bool isCullingActivated);
		public:
			bool Equal(const JSpaceSpatialOption& tar)const noexcept;
			bool SameInnerRoot(const JSpaceSpatialOption& tar)const noexcept;
			bool SameDebugRoot(const JSpaceSpatialOption& tar)const noexcept;
		public:
			bool HasInnerRoot()const noexcept;
			bool HasDebugRoot()const noexcept;
		public:
			void Store(std::wofstream& stream);
			void Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
		};
	}
}
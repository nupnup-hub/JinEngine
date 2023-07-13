#include"JSpaceSpatialOption.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Core/File/JFileIOHelper.h"

namespace JinEngine
{
	namespace Core
	{
		JSpaceSpatialOption::JSpaceSpatialOption(JUserPtr<JGameObject> innerRoot,
			JUserPtr<JGameObject> debugRoot,
			bool isSpaceSpatialActivated,
			bool isDebugActivated, 
			bool isDebugLeafOnly,
			bool isCullingActivated)
			:innerRoot(innerRoot),
			debugRoot(debugRoot),
			isSpaceSpatialActivated(isSpaceSpatialActivated),
			isDebugActivated(isDebugActivated),
			isDebugLeafOnly(isDebugLeafOnly),
			isCullingActivated(isCullingActivated)
		{}
		bool JSpaceSpatialOption::Equal(const JSpaceSpatialOption& tar)const noexcept
		{
			return isSpaceSpatialActivated == tar.isSpaceSpatialActivated &&
				isDebugActivated == tar.isDebugActivated &&
				isDebugLeafOnly == tar.isDebugLeafOnly &&
				isCullingActivated == tar.isCullingActivated &&
				SameInnerRoot(tar) && SameDebugRoot(tar);
		}
		bool JSpaceSpatialOption::SameInnerRoot(const JSpaceSpatialOption& tar)const noexcept
		{
			bool isSameInnerRoot = false;
			bool hasInnerRoot = innerRoot != nullptr && tar.innerRoot != nullptr;
			if (hasInnerRoot)
			{
				if (innerRoot->GetGuid() == tar.innerRoot->GetGuid())
					isSameInnerRoot = true;
			}
			else
			{
				if (innerRoot == nullptr && tar.innerRoot == nullptr)
					isSameInnerRoot = true;
			}
			return isSameInnerRoot;
		}
		bool JSpaceSpatialOption::SameDebugRoot(const JSpaceSpatialOption& tar)const noexcept
		{
			bool isSameDebugRoot = false;
			bool hasDebugRoot = debugRoot != nullptr && tar.debugRoot != nullptr;
			if (hasDebugRoot)
			{
				if (debugRoot->GetGuid() == tar.debugRoot->GetGuid())
					isSameDebugRoot = true;
			}
			else
			{
				if (debugRoot == nullptr && tar.debugRoot == nullptr)
					isSameDebugRoot = true;
			}
			return isSameDebugRoot;
		}
		bool JSpaceSpatialOption::HasInnerRoot()const noexcept
		{
			return innerRoot != nullptr && innerRoot.IsValid();
		}
		bool JSpaceSpatialOption::HasDebugRoot()const noexcept
		{
			return debugRoot != nullptr && debugRoot.IsValid();
		}
		void JSpaceSpatialOption::Store(std::wofstream& stream)
		{
			if (!stream.is_open())
				return;

			JFileIOHelper::StoreAtomicData(stream, L"IsSpaceSpatialActivated:", isSpaceSpatialActivated);
			JFileIOHelper::StoreAtomicData(stream, L"IsDebugActivated:", isDebugActivated);
			JFileIOHelper::StoreAtomicData(stream, L"IsDebugLeafOnly:", isDebugLeafOnly);
			JFileIOHelper::StoreAtomicData(stream, L"IsCullingActivated:", isCullingActivated);
			if (innerRoot.IsValid())
			{
				JFileIOHelper::StoreAtomicData(stream, L"hasInnerRoot:", true);
				JFileIOHelper::StoreAtomicData(stream, L"innerGuid:", innerRoot->GetGuid());
			}
			else
			{
				JFileIOHelper::StoreAtomicData(stream, L"hasInnerRoot:", false);
				JFileIOHelper::StoreAtomicData(stream, L"innerGuid:", 0);
			}
		}
		void JSpaceSpatialOption::Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
		{
			if (!stream.is_open() || stream.eof())
				return;

			JFileIOHelper::LoadAtomicData(stream, isSpaceSpatialActivated);
			JFileIOHelper::LoadAtomicData(stream, isDebugActivated);
			JFileIOHelper::LoadAtomicData(stream, isDebugLeafOnly);
			JFileIOHelper::LoadAtomicData(stream, isCullingActivated);
			JFileIOHelper::LoadAtomicData(stream, hasInnerRoot);
			JFileIOHelper::LoadAtomicData(stream, innerRootGuid);
		}
	}
}
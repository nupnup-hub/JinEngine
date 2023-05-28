#pragma once
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_CULLING_TYPE
		{
			FRUSTUM,
			OCCLUSION,
			COUNT
		}; 
		 
		class JCullingTypeData
		{
		private:
			using CullingPtr = void(*)(void*, const uint, const bool)noexcept;
			using IsCullingPtr = bool(*)(void*, const uint)noexcept;
		public:
			static bool IsOwnResultData(const J_CULLING_TYPE type)noexcept
			{
				switch (type)
				{
				case JinEngine::Graphic::J_CULLING_TYPE::FRUSTUM:
					return true;
				case JinEngine::Graphic::J_CULLING_TYPE::OCCLUSION:
					return false;
				default:
					break;
				}
			}
			static size_t GetResultDataSize(const J_CULLING_TYPE type)noexcept
			{
				switch (type)
				{
				case JinEngine::Graphic::J_CULLING_TYPE::FRUSTUM:
					return sizeof(bool);
				case JinEngine::Graphic::J_CULLING_TYPE::OCCLUSION:
					return sizeof(float);
				default:
					break;
				}
			}
			static CullingPtr GetCullingPtr(const J_CULLING_TYPE type)noexcept
			{
				switch (type)
				{
				case JinEngine::Graphic::J_CULLING_TYPE::FRUSTUM:
					return [](void* ptr, const uint index, const bool value)noexcept {static_cast<bool*>(ptr)[index] = value; };
				case JinEngine::Graphic::J_CULLING_TYPE::OCCLUSION:
					return [](void* ptr, const uint index, const bool value)noexcept {static_cast<float*>(ptr)[index] = value; };
				default:
					return nullptr;
				}
			}
			static IsCullingPtr GetIsCullingPtr(const J_CULLING_TYPE type)noexcept
			{
				switch (type)
				{
				case JinEngine::Graphic::J_CULLING_TYPE::FRUSTUM:
					return [](void* ptr, const uint index)noexcept {return static_cast<bool*>(ptr)[index]; };
				case JinEngine::Graphic::J_CULLING_TYPE::OCCLUSION:
					return [](void* ptr, const uint index)noexcept -> bool {return static_cast<float*>(ptr)[index]; };
				default:
					return nullptr;
				}
			}
		};
	}
}
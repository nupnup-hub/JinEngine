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
#include"JObjectType.h"
#include"Component/JComponentType.h"
#include"Component/Light/JLightType.h"
#include"Resource/JResourceObjectType.h"
#include"../Core/Reflection/JReflection.h"
#include"../Core/Geometry/Mesh/JMeshType.h"

namespace JinEngine
{ 
	namespace Private
	{
		template<typename T, T _type, uint variationCount>
		struct TypeVariation
		{
		public:
			using Type = T;
		public:
			static constexpr int value = variationCount;
			static constexpr T type = _type;
		};

		template<typename ...Param>
		struct TotalTypeVariation {};
		template<typename T, typename ...Param>
		struct TotalTypeVariation<T, Param...>
		{
		public:
			static constexpr int value = T::value + TotalTypeVariation<Param...>::value;
			static constexpr typename T::Type type = T::type;
		};
		template<>
		struct TotalTypeVariation<>
		{
		public:
			static constexpr int value = 0;
		};

		template<int index, typename T>
		struct TypeVariationOffset
		{
		public:
			static constexpr UniqueIndex value = -INT_MAX;
		};
		template<int index, typename T, typename ...Param>
		struct TypeVariationOffset<index, TotalTypeVariation<T, Param...>>
		{
		public:
			static constexpr UniqueIndex value = T::value + TypeVariationOffset<index - 1, TotalTypeVariation<Param...>>::value;
		};
		template<typename T>
		struct TypeVariationOffset<0, T>
		{
		public:
			static constexpr UniqueIndex value = 0;
		};

		template<typename T, T type, typename U>
		struct TypeVariationIndex {};

		template<typename T, T type, uint variationCount, typename ...Param>
		struct TypeVariationIndex<T, type, TotalTypeVariation<TypeVariation<T, type, variationCount>, Param...>>
		{
		public:
			enum { value = 0 };
		};

		template<typename T, T type, typename U, typename ...Param>
		struct TypeVariationIndex<T, type, TotalTypeVariation<U, Param...>>
		{
		private:
			enum { temp = TypeVariationIndex<T, type, TotalTypeVariation<Param...>>::value };
		public:
			enum { value = (temp == -1 ? -1 : temp + 1) };
		};
		template<typename T, T type>
		struct TypeVariationIndex<T, type, Core::JEmptyType>
		{
		public:
			enum { value = -1 };
		}; 
	}

	using AnimatorVariation = Private::TypeVariation<J_COMPONENT_TYPE, J_COMPONENT_TYPE::ENGINE_ANIMATOR, 1>;
	using BehaviorVariation = Private::TypeVariation<J_COMPONENT_TYPE, J_COMPONENT_TYPE::USER_BEHAVIOR, 1>;
	using CameraVariation = Private::TypeVariation<J_COMPONENT_TYPE, J_COMPONENT_TYPE::ENGINE_CAMERA, 1>;
	using LightVariation = Private::TypeVariation<J_COMPONENT_TYPE, J_COMPONENT_TYPE::ENGINE_LIGHT, uint(J_LIGHT_TYPE::COUNT)>;
	using RenderItemVariation = Private::TypeVariation<J_COMPONENT_TYPE, J_COMPONENT_TYPE::ENGINE_RENDERITEM, 1>;
	using TransformVariation = Private::TypeVariation<J_COMPONENT_TYPE, J_COMPONENT_TYPE::ENGINE_TRANSFORM, 1>;

	using MeshVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::MESH, uint(Core::J_MESHGEOMETRY_TYPE::COUNT)>;
	using MaterialVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::MATERIAL, 1>;
	using TextureVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::TEXTURE, 1>;
	using ShaderVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::SHADER, 1>;
	using SceneVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::SCENE, 1>;
	using ScriptVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::SCRIPT, 1>;
	using SkeletonVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::SKELETON, 1>;
	using AnimationClipVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::ANIMATION_CLIP, 1>;
	using AnimationControllerVariation = Private::TypeVariation<J_RESOURCE_TYPE, J_RESOURCE_TYPE::ANIMATION_CONTROLLER, 1>;
	 
	//새로운 하위타입 추가시 반드시 변경사항을 적용해야함.
	using CompVariation = Private::TotalTypeVariation<AnimatorVariation,
		BehaviorVariation,
		CameraVariation,
		LightVariation,
		RenderItemVariation,
		TransformVariation>;

	using ResourceVariation = Private::TotalTypeVariation<MeshVariation,
		MaterialVariation,
		TextureVariation,
		ShaderVariation,
		SceneVariation,
		ScriptVariation,
		SkeletonVariation,
		AnimationClipVariation,
		AnimationControllerVariation>;

	//always one
	static const UniqueIndex gameObjectTypeCount = 1;
	static const UniqueIndex directoryTypeCount = 1;
	static const UniqueIndex fileTypeCount = 1;

	static constexpr uint totalCompVariation = CompVariation::value;
	static constexpr uint totalResourceVariation = ResourceVariation::value;
	static constexpr uint totalCompAndResourceVariation = totalCompVariation + totalResourceVariation;

	template<J_COMPONENT_TYPE type>
	static constexpr UniqueIndex compVariationIndex = Private::TypeVariationIndex<J_COMPONENT_TYPE, type, CompVariation>::value;

	template<J_RESOURCE_TYPE type>
	static constexpr UniqueIndex resourceVariationIndex = Private::TypeVariationIndex<J_RESOURCE_TYPE, type, ResourceVariation>::value;

	template<J_COMPONENT_TYPE type>
	static constexpr UniqueIndex compVariationOffset = Private::TypeVariationOffset<compVariationIndex<type>, CompVariation>::value;

	template<J_RESOURCE_TYPE type>
	static constexpr UniqueIndex resourceVariationOffset = Private::TypeVariationOffset<resourceVariationIndex<type>, ResourceVariation>::value;

	template<J_COMPONENT_TYPE type>
	static constexpr UniqueIndex ConvertCompUniqueIndex(const UniqueIndex localIndex = 0)
	{
		return compVariationOffset<type> +localIndex;
	}
	template<J_COMPONENT_TYPE type, typename LocalType>
	static constexpr UniqueIndex ConvertCompUniqueIndex(const LocalType localType)
	{
		return compVariationOffset<type> +(UniqueIndex)localType;
	} 
	template<J_RESOURCE_TYPE type>
	static constexpr UniqueIndex ConvertResourceUniqueIndex(const UniqueIndex localIndex = 0)
	{
		return resourceVariationOffset<type> +localIndex;
	}
	template<J_RESOURCE_TYPE type, typename LocalType>
	static constexpr UniqueIndex ConvertResourceUniqueIndex(const LocalType localType)
	{
		return resourceVariationOffset<type> +(UniqueIndex)localType;
	}

	static UniqueIndex ConvertUniqueIndex(const J_COMPONENT_TYPE type, const UniqueIndex localIndex)
	{
		switch (type)
		{
		case JinEngine::J_COMPONENT_TYPE::USER_BEHAVIOR:
			return compVariationOffset<J_COMPONENT_TYPE::USER_BEHAVIOR> +localIndex;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_ANIMATOR:
			return compVariationOffset<J_COMPONENT_TYPE::ENGINE_ANIMATOR> +localIndex;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_CAMERA:
			return compVariationOffset<J_COMPONENT_TYPE::ENGINE_CAMERA> +localIndex;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_LIGHT:
			return compVariationOffset<J_COMPONENT_TYPE::ENGINE_LIGHT> +localIndex;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_RENDERITEM:
			return compVariationOffset<J_COMPONENT_TYPE::ENGINE_RENDERITEM> +localIndex;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_TRANSFORM:
			return compVariationOffset<J_COMPONENT_TYPE::ENGINE_TRANSFORM> +localIndex;
		default:
			return invalidIndex;
		}
	}
	static UniqueIndex ConvertUniqueIndex(const J_RESOURCE_TYPE type, const UniqueIndex localIndex)
	{
		switch (type)
		{
		case JinEngine::J_RESOURCE_TYPE::MESH:
			return resourceVariationOffset<J_RESOURCE_TYPE::MESH> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::MATERIAL:
			return resourceVariationOffset<J_RESOURCE_TYPE::MATERIAL> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::TEXTURE:
			return resourceVariationOffset<J_RESOURCE_TYPE::TEXTURE> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::SHADER:
			return resourceVariationOffset<J_RESOURCE_TYPE::SHADER> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::SCENE:
			return resourceVariationOffset<J_RESOURCE_TYPE::SCENE> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::SCRIPT:
			return resourceVariationOffset<J_RESOURCE_TYPE::SCRIPT> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::SKELETON:
			return resourceVariationOffset<J_RESOURCE_TYPE::SKELETON> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::ANIMATION_CLIP:
			return resourceVariationOffset<J_RESOURCE_TYPE::ANIMATION_CLIP> +localIndex;
		case JinEngine::J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
			return resourceVariationOffset<J_RESOURCE_TYPE::ANIMATION_CONTROLLER> +localIndex;
		default:
			return invalidIndex;
		}
	}
}


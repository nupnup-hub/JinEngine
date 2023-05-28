#pragma once  
#include<functional>  
#include<type_traits> 
#include"JResourceObjectType.h"   
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h"
#include"Shader/JShaderFunctionEnum.h"
#include"Texture/JDefaulTextureType.h"   
#include"../../Core/Func/Functor/JFunctor.h"

namespace JinEngine
{  
	class JMeshGeometry;
	class JMaterial;
	class JTexture;
	class JShader;     
	class JDirectory; 
	class JResourceObject;
	class JResourceManagerPrivate;

	namespace Core
	{
		template<typename T>class JCreateUsingNew;
	}
	class JResourceManager final
	{
	private:
		template<typename T>friend class Core::JCreateUsingNew;
	private:
		friend class JResourceManagerPrivate;
		class JResourceManagerImpl;
	private:
		std::unique_ptr<JResourceManagerImpl> impl;
	public: 
		//has possibility of load resource
		JUserPtr<JMeshGeometry> GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)const noexcept;
		JUserPtr<JMaterial> GetDefaultMaterial(const J_DEFAULT_MATERIAL type)const noexcept;
		JUserPtr<JTexture> GetDefaultTexture(const J_DEFAULT_TEXTURE type)const noexcept;
		JUserPtr<JShader> GetDefaultShader(const J_DEFAULT_GRAPHIC_SHADER type)const noexcept;
		JUserPtr<JShader> GetDefaultShader(const J_DEFAULT_COMPUTE_SHADER type)const noexcept;
		JUserPtr<JDirectory> GetDirectory(const size_t guid)const noexcept;
		JUserPtr<JDirectory> GetDirectory(const std::wstring& path)const noexcept;
		JUserPtr<JDirectory> GetProjectContentsDirectory()const noexcept;
		JUserPtr<JDirectory> GetEditorResourceDirectory()const noexcept;
		JUserPtr<JDirectory> GetActivatedDirectory()const noexcept;
	public:
		uint GetResourceCount(const Core::JTypeInfo& info)const noexcept;
		JUserPtr<JResourceObject> GetResource(const Core::JTypeInfo& info, const size_t guid)const noexcept;
		JUserPtr<JResourceObject> GetResourceByPath(const Core::JTypeInfo& info, const std::wstring& path)const  noexcept;
		JUserPtr<JResourceObject> TryGetResourceUser(const Core::JTypeInfo& info, const size_t guid) noexcept;			//has possibility of load resource
	public:
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		uint GetResourceCount()const noexcept
		{
			return GetResourceCount(T::StaticTypeInfo());
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		JUserPtr<T> GetResource(const size_t guid)const noexcept
		{  
			return Core::ConvertChildUserPtr<T>(GetResource(T::StaticTypeInfo(), guid));
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		JUserPtr<T> TryGetResourceUser(const size_t guid)noexcept
		{
			return static_cast<T*>(TryGetResourceUser(T::StaticTypeInfo(), guid));
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		JUserPtr<T> GetResourceByPath(const std::wstring& path)const noexcept
		{
			return static_cast<T*>(GetResourceByPath(T::StaticTypeInfo(), path));
		}
		template<typename T, typename ...Param, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		JUserPtr<T> GetResourceByCondition(Core::JFunctor<bool, T*, Param...>&& functor, Param... var)const noexcept
		{
			return FineResource<T>(T::StaticTypeInfo(), std::move(functor), std::forward<Param>(var)...);
		} 
		template<typename T, typename ...Param, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		JUserPtr<T> GetResourceByCondition(bool(*ptr)(T*, Param...), Param... var)const noexcept
		{
			return FineResource<T>(T::StaticTypeInfo(), ptr, std::forward<Param>(var)...);
		}
		template<typename T, typename ...Param, std::enable_if_t<std::is_base_of_v<JDirectory, T>, int> = 0>
		JUserPtr<T> GetDirectoryByCondition(Core::JFunctor<bool, T*, Param...>&& functor, Param... var)const noexcept
		{
			return FineResource<T>(T::StaticTypeInfo(), std::move(functor), std::forward<Param>(var)...);
		}
		template<typename T, typename ...Param, std::enable_if_t<std::is_base_of_v<JDirectory, T>, int> = 0>
		JUserPtr<T> GetDirectoryByCondition(bool(*ptr)(T*, Param...), Param... var)const noexcept
		{
			return FineResource<T>(T::StaticTypeInfo(), ptr, std::forward<Param>(var)...);
		} 
	private:
		template<typename T,  typename F, typename ...Param>
		JUserPtr<T> FineResource(const Core::JTypeInfo& typeInfo, F func, Param... var)const noexcept
		{
			auto vec = typeInfo.GetInstanceRawPtrVec();
			for (const auto& data : vec)
			{
				T* obj = static_cast<T*>(data);
				if (func(obj, std::forward<Param>(var)...))
					return Core::GetUserPtr(obj);
			}
			return nullptr;
		}
	private:
		JResourceManager();
		~JResourceManager();
	}; 

	using _JResourceManager = JinEngine::Core::JSingletonHolder<JResourceManager>;
}
 
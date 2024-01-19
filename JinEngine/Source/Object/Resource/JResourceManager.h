#pragma once  
#include<functional>  
#include<type_traits> 
#include"JResourceObjectType.h"   
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h"
#include"Shader/JShaderEnum.h"
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
	 
	//private interface�� impl�������� static class���� singleton�� �� �����ϴٰ� �Ǵ��ؼ�
	//���� �� ������ �ʿ��� manager class�� singleton ����
	//�߰�) �ٸ� class�� private interface ����Ϸ��� friend class ������ �ʿ��ϰ� �̴� ������Ͽ� impl class�� ������
	//�����Ѵ� ���� impl�� ���� ������ �����ϸ� �̴� �����ڿ��� ���� �ʱ�ȭ�� �ʿ���ϹǷ� static class����
	//singletone�� ä���� �����̴�. 

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
		size_t GetDefaultGuid(const J_DEFAULT_SHAPE type)const noexcept;
		size_t GetDefaultGuid(const J_DEFAULT_MATERIAL type)const noexcept;
		size_t GetDefaultGuid(const J_DEFAULT_TEXTURE type)const noexcept;
		size_t GetDefaultGuid(const J_DEFAULT_GRAPHIC_SHADER type)const noexcept;
		size_t GetDefaultGuid(const J_DEFAULT_COMPUTE_SHADER type)const noexcept;
	public:
		uint GetResourceCount(const Core::JTypeInfo& info)const noexcept;
		JUserPtr<JResourceObject> GetResource(const Core::JTypeInfo& info, const size_t guid)const noexcept;
		JUserPtr<JResourceObject> GetResourceByPath(const Core::JTypeInfo& info, const std::wstring& path)const  noexcept;
		JUserPtr<JResourceObject> TryGetResourceUser(const Core::JTypeInfo& info, const size_t guid) noexcept;		//has possibility of load resource
		JUserPtr<JResourceObject> TryGetResourceUser(const Core::JTypeInstanceSearchHint& hint) noexcept;			//has possibility of load resource
		JUserPtr<JResourceObject> TryGetResourceUser(const size_t typeGuid, const size_t objGuid) noexcept;			//has possibility of load resource
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
		/**
		* @brief search resource or directory
		*/
		template<typename T, typename ...Param>
		auto GetByCondition(Core::JFunctor<bool, T*, Param...>&& functor,
			const bool allowChildTypeSearch,
			Param... var)const noexcept
			-> std::conditional_t<std::is_base_of_v<JResourceObject, T> || std::is_base_of_v<JDirectory, T>,
			JUserPtr<T>,
			void>
		{
			return FindJResource<T>(T::StaticTypeInfo(), std::move(functor), allowChildTypeSearch, std::forward<Param>(var)...);
		} 
		/**
		* @brief search resource or directory
		*/
		template<typename T, typename ...Param>
		auto GetByCondition(bool(*ptr)(T*, Param...),
			const bool allowChildTypeSearch,
			Param... var)const noexcept
			-> std::conditional_t<std::is_base_of_v<JResourceObject, T> || std::is_base_of_v<JDirectory, T>,
			JUserPtr<T>,
			void>
		{
			return FindJResource<T>(T::StaticTypeInfo(), ptr, allowChildTypeSearch, std::forward<Param>(var)...);
		}
	private:
		template<typename T,  typename F, typename ...Param>
		JUserPtr<T> FindJResource(const Core::JTypeInfo& typeInfo, 
			F func, 
			const bool allowChildTypeSearch,
			Param... var)const noexcept
		{
			auto vec = typeInfo.GetInstanceRawPtrVec();
			for (const auto& data : vec)
			{
				T* obj = static_cast<T*>(data);
				if (func(obj, std::forward<Param>(var)...))
					return Core::GetUserPtr(obj);
			}
			if (allowChildTypeSearch)
			{
				auto childVec = typeInfo.GetChildInfo();
				for (const auto& childType : childVec)
				{
					auto childObjVec = childType->GetInstanceRawPtrVec();
					for (const auto& data : childObjVec)
					{
						T* obj = static_cast<T*>(data);
						if (func(obj, std::forward<Param>(var)...))
							return Core::GetUserPtr(obj);
					}
				}
			}
			return nullptr;
		}
	private:
		JResourceManager();
		~JResourceManager();
		JResourceManager(const JResourceManager& rhs) = delete;
		JResourceManager& operator=(const JResourceManager& rhs) = delete;
	}; 

	using _JResourceManager = JinEngine::Core::JSingletonHolder<JResourceManager>;
}
 
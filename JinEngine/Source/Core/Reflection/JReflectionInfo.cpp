#include"JReflectionInfo.h"
#include"JReflectionInfoPrivate.h"
#include"JTypeInfo.h" 
#include"JEnumInfo.h"
#include"../JCoreEssential.h"
#include"../Time/JGameTimer.h"
#include"../Utility/JCommonUtility.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Object/Resource/JResourceObjectHint.h"
#include<set>
#include<vector>  
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JReflectionInfo::JReflectionInfoImpl
		{
		private:
			using TypeVec = std::vector<JTypeInfo*>;
			using TypeMap = std::unordered_map<size_t, JTypeInfo*>; 
		private:
			using EnumVec = std::vector<JEnumInfo*>;
			using EnumMap = std::unordered_map<size_t, JEnumInfo*>;
		private:
			using TimeVec = std::vector<float>;
		private:
			struct JTypeData
			{
			public:
				TypeVec typeVec;
				TypeMap typeMap;
			};
			struct JEnumData
			{
			public:
				EnumVec enumVec;
				EnumMap enumMap;
			};
			struct JLazyData
			{
			public:
				TypeVec lazyTypeVec;
				TimeVec timeVec;
			};
		private:
			JTypeData jType; 
			JEnumData jEnum;
			JLazyData jLazy;
		private:
			int lazyUpdateIndex = 0;
		public:
			void Initialize()
			{
				static bool callOnce = false;
				if (callOnce)
					return;

				std::vector<JTypeInfo*> typeVec = GetAllTypeInfo();
				for (auto& data : typeVec)
					data->ExecuteTypeCallOnece();

				std::set<size_t> registeredType;
				JGameObject::StaticTypeInfo().RegisterAllocation();
				registeredType.emplace(JGameObject::StaticTypeInfo().TypeGuid());

				auto componentTypeVec = GetDerivedTypeInfo(JComponent::StaticTypeInfo());
				for (auto& data : componentTypeVec)
				{
					if (!data->IsAbstractType())
					{
						data->RegisterAllocation();
						data->RegisterLazyDestructionInfo();
						registeredType.emplace(data->TypeGuid());
					}
				}

				auto rTypeHintVec = RTypeCommonCall::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY);
				for (auto& hint : rTypeHintVec)
				{
					Core::JTypeInfo& typeInfo = RTypeCommonCall::CallGetTypeInfo(hint.thisType);
					auto derivedVec = GetDerivedTypeInfo(typeInfo);
					for (auto& data : derivedVec)
					{
						if (!data->IsAbstractType())
						{
							data->RegisterAllocation();
							data->RegisterLazyDestructionInfo();
							registeredType.emplace(data->TypeGuid());
						}
					}
				}

				auto idenTypeVec = GetDerivedTypeInfo(Core::JIdentifier::StaticTypeInfo());
				for (auto& data : idenTypeVec)
				{
					if (!data->IsAbstractType() && registeredType.find(data->TypeGuid()) == registeredType.end())
					{
						data->RegisterAllocation();
						data->RegisterLazyDestructionInfo();
						registeredType.emplace(data->TypeGuid());
					}
				}
				for (auto& data : typeVec)
				{
					if (!data->IsAbstractType() && registeredType.find(data->TypeGuid()) == registeredType.end())
					{
						data->RegisterAllocation();
						data->RegisterLazyDestructionInfo();
						registeredType.emplace(data->TypeGuid());
					}
					if (data->CanUseLazyDestruction())
						jLazy.lazyTypeVec.push_back(data);
				}

				jLazy.timeVec.resize(jLazy.lazyTypeVec.size());
				for (auto& data : jLazy.timeVec)
					data = 0;

				for (auto& data : typeVec)
					data->EndRegister();
				callOnce = true;
			}
			void Clear()
			{
				auto typeVec = GetDerivedTypeInfo(Core::JIdentifier::StaticTypeInfo());
				for (auto& data : typeVec)
					data->DeRegisterAllocation();
			}
		public:
			JTypeInfo* GetTypeInfo(const std::string& fullname)const noexcept
			{
				return GetTypeInfo(std::hash<std::string>{}(fullname));
			}
			JTypeInfo* GetTypeInfo(const size_t typeGuid)const noexcept
			{
				auto data = jType.typeMap.find(typeGuid);
				return data != jType.typeMap.end() ? data->second : nullptr;
			}
			JEnumInfo* GetEnumInfo(const std::string& fullname)const noexcept
			{
				return GetEnumInfo(std::hash<std::string>{}(fullname));
			}
			JEnumInfo* GetEnumInfo(const size_t enumGuid)const noexcept
			{
				auto data = jEnum.enumMap.find(enumGuid);
				return data != jEnum.enumMap.end() ? data->second : nullptr;
			}
			std::vector<JTypeInfo*> GetAllTypeInfo()const noexcept
			{
				return jType.typeVec;
			}
			std::vector<JTypeInfo*> GetDerivedTypeInfo(const JTypeInfo& baseType, const bool containBaseType = false)const noexcept
			{
				std::vector<JTypeInfo*> res;
				const uint typeCount = (uint)jType.typeVec.size();
				for (uint i = 0; i < typeCount; ++i)
				{
					if (!containBaseType && jType.typeVec[i]->IsA(baseType))
						continue;

					if (jType.typeVec[i]->IsChildOf(baseType))
						res.push_back(jType.typeVec[i]);
				}
				return res;
			}
		public:
			void SearchInstance()
			{
				const uint typeCount = (uint)jType.typeVec.size();
				for (uint i = 0; i < typeCount; ++i)
				{
					if (jType.typeVec[i]->instanceData != nullptr && jType.typeVec[i]->instanceData->classInstanceVec.size() > 0)
					{
						MessageBox(0, std::to_wstring(jType.typeVec[i]->instanceData->classInstanceVec.size()).c_str(), JCUtil::StrToWstr(jType.typeVec[i]->Name()).c_str(), 0);
					}
				}
			}
		public:
			void Update()
			{
				jLazy.lazyTypeVec[lazyUpdateIndex]->UpdateLazyDestruction(jLazy.timeVec[lazyUpdateIndex]);
				++lazyUpdateIndex;
				if (lazyUpdateIndex >= jLazy.lazyTypeVec.size())
					lazyUpdateIndex = 0;

				float deltaTime = JEngineTimer::Data().DeltaTime();
				for (auto& data : jLazy.timeVec)
					data += deltaTime;

				jLazy.timeVec[lazyUpdateIndex] = 0;
			}
		public:
			void AddType(JTypeInfo* newType)
			{
				assert(newType != nullptr);
				if (GetTypeInfo(newType->TypeGuid()) != nullptr)
					return;

				jType.typeVec.push_back(newType);
				jType.typeMap.emplace(newType->TypeGuid(), newType);
			}
			void AddEnum(JEnumInfo* newEnum)
			{
				assert(newEnum != nullptr);
				if (GetEnumInfo(newEnum->EnumGuid()) != nullptr)
					return;

				jEnum.enumVec.push_back(newEnum);
				jEnum.enumMap.emplace(newEnum->EnumGuid(), newEnum);
			}
		};

		JTypeInfo* JReflectionInfo::GetTypeInfo(const std::string& fullname)const noexcept
		{ 
			return impl->GetTypeInfo(fullname);
		} 
		JTypeInfo* JReflectionInfo::GetTypeInfo(const size_t typeGuid)const noexcept
		{
			return impl->GetTypeInfo(typeGuid);
		}
		JEnumInfo* JReflectionInfo::GetEnumInfo(const std::string& fullname)const noexcept
		{
			return impl->GetEnumInfo(fullname);
		}
		JEnumInfo* JReflectionInfo::GetEnumInfo(const size_t enumGuid)const noexcept
		{
			return impl->GetEnumInfo(enumGuid);
		}
		std::vector<JTypeInfo*> JReflectionInfo::GetAllTypeInfo()const noexcept
		{
			return impl->GetAllTypeInfo();
		}
		std::vector<JTypeInfo*> JReflectionInfo::GetDerivedTypeInfo(const JTypeInfo& baseType, const bool containBaseType)const noexcept
		{
			return impl->GetDerivedTypeInfo(baseType, containBaseType);
		}
		void JReflectionInfo::SearchInstance()
		{
			impl->SearchInstance();
		}
		JReflectionInfo::JReflectionInfo()
			:impl(std::make_unique<JReflectionInfoImpl>())
		{

		}
		JReflectionInfo::~JReflectionInfo()
		{
			impl.reset();
		}

		using TypeInterface = JReflectionInfoPrivate::TypeInterface; 
		using EnumInterface = JReflectionInfoPrivate::EnumInterface;
		using MainAccess = JReflectionInfoPrivate::MainAccess;

		void TypeInterface::AddType(JTypeInfo* newType)
		{
			_JReflectionInfo::Instance().impl->AddType(newType);
		}

		void EnumInterface::AddEnum(JEnumInfo* newEnum)
		{
			_JReflectionInfo::Instance().impl->AddEnum(newEnum);
		}

		void MainAccess::Initialize()
		{
			_JReflectionInfo::Instance().impl->Initialize();
		}
		void MainAccess::Clear()
		{
			_JReflectionInfo::Instance().impl->Clear();
		}
		void MainAccess::Update()
		{
			_JReflectionInfo::Instance().impl->Update();
		}
	}
}
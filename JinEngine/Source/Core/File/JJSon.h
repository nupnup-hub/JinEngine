#pragma once
#include"../JCoreEssential.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../../ThirdParty/jsoncpp-master/json.h"
#include<string>

namespace JinEngine
{
	namespace Core
	{
		class JJSon
		{
		public:
			struct CurrentData
			{
			public:
				Json::Value* ptr;
				Json::Value::iterator iter;
			public:
				CurrentData(Json::Value& v);
			};
		private:
			std::string path;
			Json::Value contents;
			std::stack<CurrentData> currentStack;
		private:
			const bool isContentsMemberArray; 
		public:
			JJSon(const std::string& path, const bool isContentsMemberArray = false);
			~JJSon();
		public:
			bool Load();
			bool Store();
		public:
			template<typename T>
			static bool LoadData(JJSon& json, T& data, const std::string& label = "")
			{
				if (json.currentStack.size() == 0)
					return false;

				CurrentData& current = json.currentStack.top();
				const bool isArray = current.ptr->isArray();
			 
				//label = JCUtil::EraseSideChar(label, ' ');
				Json::Value& value = isArray ? current.ptr[current.iter.index()]: (*current.ptr)[label];
				if (value.isNull())
					return false;

				if (isArray)
					++current.iter;

				if constexpr (std::is_same_v<T, bool>)
				{
					data = static_cast<T>(value.asBool());
					return true;
				}
				else if constexpr (std::is_integral_v<T>)
				{
					if constexpr (std::is_unsigned_v<T>)
					{
						if constexpr (std::is_same_v<T, size_t>)
							data = static_cast<T>(value.asUInt64());
						else
							data = static_cast<T>(value.asUInt());
					}
					else
					{
						if constexpr (std::is_same_v<T, int64>)
							data = static_cast<T>(value.asInt64());
						else
							data = static_cast<T>(value.asInt());
					}
					return true;
				}
				else if constexpr (std::is_floating_point_v<T>)
				{
					data = static_cast<T>(value.asFloat());
					return true;
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					data = static_cast<T>(value.asString());
					return true;
				}
				else if constexpr (std::is_enum_v<T>)
				{
					data = static_cast<T>(value.asInt());
					return true;
				}
				else
					return false;
			}
			template<typename T>
			static bool StoreData(JJSon& json,  const T data, const std::string& label = "")
			{
				if (json.currentStack.size() == 0)
					return false;

				CurrentData& current = json.currentStack.top();
				const bool isArray = current.ptr->isArray();
				 
				static constexpr bool canStore = std::is_same_v<T, bool> ||
					std::is_integral_v<T> ||
					std::is_floating_point_v<T> ||
					std::is_same_v<T, std::string> ||
					std::is_enum_v<T>;

				if constexpr (canStore)
				{
					if constexpr (std::is_enum_v<T>)
					{
						if (isArray)
						{
							current.ptr->append(static_cast<int>(data));
							++current.iter;
						}
						else
							(*current.ptr)[label] = static_cast<int>(data);
					}
					else
					{
						if (isArray)
						{
							current.ptr->append(data);
							++current.iter;
						}
						else
							(*current.ptr)[label] = data;
					}
					return true;
				}
				else
					return false;
			}
		public:
			/*
			template<typename T>
			void PushValue(T data, const std::string& key = "")
			{ 
				if (StoreData(*this, data, key))
				{
					if (currentStack.top().value.isArray())
					{
						--currentStack.top().iter;
						currentStack.emplace(currentStack.top().ptr[currentStack.top().iter.index()]);
						++currentStack.top().iter;
					}
					else
						currentStack.emplace(currentStack.top().value[key]);
				} 
			}  
			*/
			bool PushArrayOwner(const std::string& key = "");
			bool PushArrayMember();
			bool PushMapMember(const std::string& key);
			/**
			* @brief for load state
			*/
			bool PushExistStack(const std::string& key = ""); 
			bool PopStack();
		public:
			uint GetCurrentMemberCount()const noexcept;
		public:
			bool IsCurrentHasArrayContainer()const noexcept;
			bool HasCurrentStack()const noexcept;
		};
	}
}
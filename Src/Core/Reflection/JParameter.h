#pragma once
#include"../JDataType.h"
#include"../../Utility/JTypeUtility.h" 
#include"JParameterType.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			//Unsupported
			//Array To Ptr
			//Ptr to Arry 

			template<bool isConst, bool isPtr, bool isArr, bool isLvalueRef, bool isRvalueRef, typename T>
			struct JParameterConvertible;

			//Normal
			//Left Ref
			//Right Ref
			//Pointer
			//Const   

			//Normal
			template<typename T>
			struct JParameterConvertible<false, false, false, false, false, T>
			{
			public:
				static constexpr bool value = !IsPointer_V<T> && !IsArray_V<T>;
			};

			//Const Normal
			template<typename T>
			struct JParameterConvertible<true, false, false, false, false, T>
			{
			public:
				static constexpr bool value = !IsPointer_V<T> && !IsArray_V<T>;
			};

			//Pointer
			template<typename T>
			struct JParameterConvertible<false, true, false, false, false, T>
			{
			public:
				static constexpr bool value = IsPointer_V<T> && !IsConst_V<T>;
			};

			//LeftRef
			template<typename T>
			struct JParameterConvertible<false, false, false, true, false, T>
			{
			public:
				static constexpr bool value = IsLvalueRef_V<T> && !IsConst_V<T> && !IsPointer_V<T> && !IsArray_V<T>;
			};

			//RightRef
			template<typename T>
			struct JParameterConvertible<false, false, false, false, true, T>
			{
			public:
				static constexpr bool value = !IsLvalueRef_V<T> && !IsConst_V<T> && !IsPointer_V<T> && !IsArray_V<T>;
			};

			//Const Pointer
			template<typename T>
			struct JParameterConvertible<true, true, false, false, false, T>
			{
			public:
				static constexpr bool value = IsPointer_V<T>;
			};

			//Const LeftRef
			template<typename T>
			struct JParameterConvertible<true, false, false, true, false, T>
			{
			public:
				static constexpr bool value = !IsPointer_V<T> && !IsArray_V<T>;
			};

			//Const RightRef
			template<typename T>
			struct JParameterConvertible<true, false, false, false, true, T>
			{
			public:
				static constexpr bool value = !IsLvalueRef_V<T> && !IsPointer_V<T> && !IsArray_V<T>;
			};

			//Pointer LeftRef
			template<typename T>
			struct JParameterConvertible<false, true, false, true, false, T>
			{
			public:
				static constexpr bool value = IsLvalueRef_V<T> && !IsConst_V<T> && IsPointer_V<T>;
			};

			//Pointer RightRef
			template<typename T>
			struct JParameterConvertible<false, true, false, false, true, T>
			{
			public:
				static constexpr bool value = !IsLvalueRef_V<T> && !IsConst_V<T> && IsPointer_V<T>;
			};

			//Const Pointer LeftRef
			template<typename T>
			struct JParameterConvertible<true, true, false, true, false, T>
			{
			public:
				static constexpr bool value = IsLvalueRef_V<T> && IsConst_V<T> && IsPointer_V<T>;
			};

			//Const Pointer RightRef
			template<typename T>
			struct JParameterConvertible<true, true, false, false, true, T>
			{
			public:
				static constexpr bool value = (!IsLvalueRef_V<T> || !IsConst_V<T>) && IsPointer_V<T>;
			};

			//Array
			template<typename T>
			struct JParameterConvertible<false, false, true, false, false, T>
			{
				//static constexpr bool value = IsPointer_V<T> && !IsConst_V<T>;
			public:
				static constexpr bool value = IsArray_V<T> && !IsConst_V<T>;
			};

			//Const Array
			template<typename T>
			struct JParameterConvertible<true, false, true, false, false, T>
			{
			public:
				static constexpr bool value = IsArray_V<T>;
			};

			//Left Array
			template<typename T>
			struct JParameterConvertible<false, false, true, true, false, T>
			{
			public:
				static constexpr bool value = IsLvalueRef_V<T> && IsArray_V<T> && !IsConst_V<T>;
			};

			//Left Const Array
			template<typename T>
			struct JParameterConvertible<true, false, true, true, false, T>
			{
			public:
				static constexpr bool value = IsRefernce_V<T> && IsArray_V<T>;
			};

			//Right Array
			template<typename T>
			struct JParameterConvertible<false, false, true, false, true, T>
			{
			public:
				static constexpr bool value = !IsLvalueRef_V<T> && IsArray_V<T> && !IsConst_V<T>;
			};

			//Right Const Array
			template<typename T>
			struct JParameterConvertible<true, false, true, false, true, T>
			{
			public:
				static constexpr bool value = !IsLvalueRef_V<T> && IsArray_V<T>;
			};

			//
			template<bool isConst, bool isPtr, bool isArr, bool isLvalueRef, bool isRvalueRef, typename T>
			struct JParameterConvert;

			//1. Normal
			template<typename T>
			struct JParameterConvert<false, false, false, false, false, T>
			{
			public:
				using Success = RemoveAll_T<T>;
				using Fail = T;
				using Convertible = JParameterConvertible<false, false, false, false, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//2. Const Normal
			template<typename T>
			struct JParameterConvert<true, false, false, false, false, T>
			{
			public:
				using Success = std::add_const_t<RemoveAll_T<T>>;
				using Fail = T;
				using Convertible = JParameterConvertible<true, false, false, false, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//3. Pointer
			template<typename T>
			struct JParameterConvert<false, true, false, false, false, T>
			{
			public:
				static constexpr uint8 ptrCount = std::is_array_v<T> ? 1 : PointerCount_V<T>;
				using Success = typename AddPointer_T<ptrCount, RemoveAll_T<T>>;
				using Fail = T;
				using Convertible = JParameterConvertible<false, true, false, false, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//4. LeftRef
			template<typename T>
			struct JParameterConvert<false, false, false, true, false, T>
			{
			public:
				using Success = typename std::add_lvalue_reference_t<RemoveAll_T<T>>;
				using Fail = T;
				using Convertible = JParameterConvertible<false, false, false, true, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//5. RightRef
			template<typename T>
			struct JParameterConvert<false, false, false, false, true, T>
			{
			public:
				using Success = typename std::add_rvalue_reference_t<RemoveAll_T<T>>;
				using Fail = T;
				using Convertible = JParameterConvertible<false, false, false, false, true, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//6. Const Pointer
			template<typename T>
			struct JParameterConvert<true, true, false, false, false, T>
			{
			public:
				static constexpr uint8 ptrCount = std::is_array_v<T> ? 1 : PointerCount_V<T>;
				using Success = typename std::add_const_t<AddPointer_T<ptrCount, RemoveAll_T<T>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<true, true, false, false, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//7. Const LeftRef
			template<typename T>
			struct JParameterConvert<true, false, false, true, false, T>
			{
			public:
				using Success = typename std::add_const_t<std::add_lvalue_reference_t<RemoveAll_T<T>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<true, false, false, true, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//8. Const RightRef
			template<typename T>
			struct JParameterConvert<true, false, false, false, true, T>
			{
			public:
				using Success = typename std::add_const_t<std::add_rvalue_reference_t<RemoveAll_T<T>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<true, false, false, false, true, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//9. Pointer LeftRef
			template<typename T>
			struct JParameterConvert<false, true, false, true, false, T>
			{
			public:
				static constexpr uint8 ptrCount = PointerCount_V<std::remove_reference_t<T>>;
				using Success = typename std::add_lvalue_reference_t<AddPointer_T<ptrCount, RemoveAll_T<T>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<false, true, false, true, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//10. Pointer RightRef
			template<typename T>
			struct JParameterConvert<false, true, false, false, true, T>
			{
			public:
				static constexpr uint8 ptrCount = PointerCount_V<std::remove_reference_t<T>>;
				using Success = typename std::add_rvalue_reference_t<AddPointer_T<ptrCount, RemoveAll_T<T>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<false, true, false, false, true, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//11. Const Pointer LeftRef
			template<typename T>
			struct JParameterConvert<true, true, false, true, false, T>
			{
			public:
				static constexpr uint8 ptrCount = PointerCount_V<T>;
				using Success = typename std::add_const_t<std::add_lvalue_reference_t< AddPointer_T<ptrCount, RemoveAll_T<T>>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<true, true, false, true, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//12. Const Pointer RightRef
			template<typename T>
			struct JParameterConvert<true, true, false, false, true, T>
			{
			public:
				static constexpr uint8 ptrCount = std::is_array_v<T> ? 1 : PointerCount_V<T>;
				using Success = typename std::add_const_t<std::add_rvalue_reference_t<AddPointer_T<ptrCount, RemoveAll_T<T>>>>;
				using Fail = T;
				using Convertible = JParameterConvertible<true, true, false, false, true, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//13. Array
			template<typename T>
			struct JParameterConvert<false, false, true, false, false, T>
			{
			public:
				using Success = T;
				using Fail = T;
				using Convertible = JParameterConvertible<false, false, true, false, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//14. Const Array
			template<typename T>
			struct JParameterConvert<true, false, true, false, false, T>
			{
			public:
				using Success = T;
				using Fail = T;
				using Convertible = JParameterConvertible<true, false, true, false, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//15. Left Array
			template<typename T>
			struct JParameterConvert<false, false, true, true, false, T>
			{
			public:
				using Success = T;
				using Fail = T;
				using Convertible = JParameterConvertible<false, false, true, true, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};
			//16. Const Left Array
			template<typename T>
			struct JParameterConvert<true, false, true, true, false, T>
			{
			public:
				using Success = T;
				using Fail = T;
				using Convertible = JParameterConvertible<true, false, true, true, false, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};

			//17. Right Array
			template<typename T>
			struct JParameterConvert<false, false, true, false, true, T>
			{
			public:
				using Success = T;
				using Fail = T;
				using Convertible = JParameterConvertible<false, false, true, false, true, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};
			//18. Const Right Array
			template<typename T>
			struct JParameterConvert<true, false, true, false, true, T>
			{
			public:
				using Success = T;
				using Fail = T;
				using Convertible = JParameterConvertible<true, false, true, false, true, T>;
				using Type = typename JSelect<Convertible::value>::template Result<Success, Fail>;
			};
			template<typename T>
			J_PARAMETER_TYPE GetJDataEnum()
			{
				if constexpr (std::is_integral_v<T>)
				{
					if constexpr (std::is_unsigned_v<T>)
						return J_PARAMETER_TYPE::UInt;
					else
						return J_PARAMETER_TYPE::Int;
				}
				else if constexpr (std::is_floating_point_v<T>)
					return J_PARAMETER_TYPE::Float;
				else if constexpr (std::is_same_v<char, T>)
					return J_PARAMETER_TYPE::Char;
				else if constexpr (std::is_same_v<std::string, T>)
					return J_PARAMETER_TYPE::String;
				else if constexpr (std::is_class_v<T>)
					return J_PARAMETER_TYPE::Class;
				else if constexpr (std::is_enum_v<T>)
					return J_PARAMETER_TYPE::Enum;
				else if constexpr (std::is_void_v<T>)
					return J_PARAMETER_TYPE::Void;
				else
					return J_PARAMETER_TYPE::UnKnown;
			}
		}
		struct JParameterHint
		{
		public:
			const std::string name;
			const bool isConst;
			const bool isPtr;
			const bool isArray;
			const bool isLvalueReference;
			const bool isRvalueReference;
			const uint8 dimension;
			const J_PARAMETER_TYPE jDataEnum;
			const size_t arrLastDimCount;
		public:
			JParameterHint(const std::string name,
				const bool isConst,
				const bool isPtr,
				const bool isArray,
				const bool isLvalueReference,
				const bool isRvalueReference,
				const uint8 dimension,
				const J_PARAMETER_TYPE jDataEnum,
				const size_t arrLastDimCount)
				: name(name),
				isConst(isConst),
				isPtr(isPtr),
				isArray(isArray),
				isLvalueReference(isLvalueReference),
				isRvalueReference(isRvalueReference),
				dimension(dimension),
				jDataEnum(jDataEnum),
				arrLastDimCount(arrLastDimCount)
			{ }

			template<typename T>
			bool IsConvertible()const noexcept
			{
				if (!isConst && !isPtr && !isArray && !isLvalueReference && !isRvalueReference)
					return JParameterConvert<false, false, false, false, false, T>::Convertible::value;
				else if (isConst && !isPtr && !isArray && !isLvalueReference && !isRvalueReference)
					return JParameterConvert<true, false, false, false, false, T>::Convertible::value;
				else if (!isConst && isPtr && !isArray && !isLvalueReference && !isRvalueReference && dimension == PointerCount_V<T>)
					return JParameterConvert<false, true, false, false, false, T>::Convertible::value;
				else if (!isConst && !isPtr && !isArray && isLvalueReference && !isRvalueReference)
					return JParameterConvert<false, false, false, true, false, T>::Convertible::value;
				else if (!isConst && !isPtr && !isArray && !isLvalueReference && isRvalueReference)
					return JParameterConvert<false, false, false, false, true, T>::Convertible::value;
				else if (isConst && isPtr && !isArray && !isLvalueReference && !isRvalueReference && dimension == PointerCount_V<T>)
					return JParameterConvert<true, true, false, false, false, T>::Convertible::value;
				else if (isConst && !isPtr && !isArray && isLvalueReference && !isRvalueReference)
					return JParameterConvert<true, false, false, true, false, T>::Convertible::value;
				else if (isConst && !isPtr && !isArray && !isLvalueReference && isRvalueReference)
					return JParameterConvert<true, false, false, false, true, T>::Convertible::value;
				else if (!isConst && isPtr && !isArray && isLvalueReference && !isRvalueReference && dimension == PointerCount_V<T>)
					return JParameterConvert<false, true, false, true, false, T>::Convertible::value;
				else if (!isConst && isPtr && !isArray && !isLvalueReference && isRvalueReference && dimension == PointerCount_V<T>)
					return JParameterConvert<false, true, false, false, true, T>::Convertible::value;
				else if (isConst && isPtr && !isArray && isLvalueReference && !isRvalueReference && dimension == PointerCount_V<T>)
					return JParameterConvert<true, true, false, true, false, T>::Convertible::value;
				else if (isConst && isPtr && !isArray && !isLvalueReference && isRvalueReference && dimension == PointerCount_V<T>)
					return JParameterConvert<true, true, false, false, true, T>::Convertible::value;
				else if (!isConst && !isPtr && isArray && !isLvalueReference && !isRvalueReference &&
					dimension > 0 && dimension == ArrayDimension_V<T> && arrLastDimCount == ArrayLastDimensionCount_V<T>)
					return JParameterConvert<false, false, true, false, false, T>::Convertible::value;
				else if (isConst && !isPtr && isArray && !isLvalueReference && !isRvalueReference &&
					dimension > 0 && dimension == ArrayDimension_V<T> && arrLastDimCount == ArrayLastDimensionCount_V<T>)
					return JParameterConvert<true, false, true, false, false, T>::Convertible::value;
				else if (!isConst && !isPtr && isArray && isLvalueReference && !isRvalueReference &&
					dimension > 0 && dimension == ArrayDimension_V<T> && arrLastDimCount == ArrayLastDimensionCount_V<T>)
					return JParameterConvert<false, false, true, true, false, T>::Convertible::value;
				else if (isConst && !isPtr && isArray && isLvalueReference && !isRvalueReference &&
					dimension > 0 && dimension == ArrayDimension_V<T> && arrLastDimCount == ArrayLastDimensionCount_V<T>)
					return JParameterConvert<true, false, true, true, false, T>::Convertible::value;
				else if (!isConst && !isPtr && isArray && !isLvalueReference && isRvalueReference &&
					dimension > 0 && dimension == ArrayDimension_V<T> && arrLastDimCount == ArrayLastDimensionCount_V<T>)
					return JParameterConvert<false, false, true, false, true, T>::Convertible::value;
				else if (isConst && !isPtr && isArray && !isLvalueReference && isRvalueReference &&
					dimension > 0 && dimension == ArrayDimension_V<T> && arrLastDimCount == ArrayLastDimensionCount_V<T>)
					return JParameterConvert<true, false, true, false, true, T>::Convertible::value;
				else
					return false;
			}
		};

		template<typename T>
		static std::string GetTypeName()
		{
			std::string ret;
			if constexpr (IsConst_V<T>)
				ret += "const ";

			if constexpr (IsPointer_V<T>)
				ret += "*";

			if constexpr (std::is_rvalue_reference_v<T>)
				ret += "&&";

			if constexpr (std::is_lvalue_reference_v<T>)
				ret += "&";

			if constexpr (IsArray_V<T>)
				ret += "Arr";

			ret += typeid(T).name();
			return ret;
		}

		template<typename ...Param>
		static std::string GetParamNames(const char fence)
		{
			auto AddParamName = [](std::string& sum, const std::string add, const char fence)
			{
				sum += add + fence;
			};
			std::string sum;
			((AddParamName(sum, GetTypeName<Param>(), fence)), ...);
			return sum;
		}

		static void ClassifyParamName(const uint count, std::vector<std::string>& vec, const std::string& funcPtrName)
		{
			size_t index = funcPtrName.find_first_of(')');
			std::string copyName = funcPtrName.substr(index);
			index = copyName.find_first_of('(');
			copyName = copyName.substr(index + 1);

			for (uint i = 0; i < count; ++i)
			{
				size_t edindex = copyName.find_first_of(',');
				if (edindex == -1)
				{
					edindex = copyName.find_last_of(')');
					if (edindex == -1)
						edindex = copyName.find_last_of('>');
					vec.emplace_back(copyName.substr(0, edindex));
					break;
				}
				else
				{
					vec.emplace_back(copyName.substr(0, edindex));
					copyName = copyName.substr(edindex + 1);
				}
			}
		}

		template<typename T>
		static JParameterHint CreateParameterHint(const std::string& name)
		{
			static const std::string constStr = " const ";
			bool isConst = false;
			bool isPtr = false;
			bool isArray = false;
			bool isLref = false;
			bool isRref = false;
			uint8 dimension = 0;
			J_PARAMETER_TYPE jDataEnum = J_PARAMETER_TYPE::UnKnown;
			size_t arrLastDimCount = 0;

			if constexpr (IsPointer_V<T>)
			{
				isPtr = true;
				dimension = PointerCount_V<T>;
			}

			if constexpr (IsArray_V<T>)
			{
				isArray = true;
				dimension = ArrayDimension_V<T>;
				arrLastDimCount = ArrayLastDimensionCount_V<T>;
			}

			if constexpr (IsConst_V<T>)
				isConst = true;
			else
			{
				if (isPtr)
				{
					size_t ptrSymbolIndex = name.find_first_not_of('*');
					std::string forward = name.substr(0, ptrSymbolIndex);
					std::string back = name.substr(ptrSymbolIndex + 1);
					size_t fconstIndex = forward.find(constStr);
					size_t bconstIndex = back.find(constStr);

					if (fconstIndex != forward.npos)
						isConst = true;
				}
				else
				{
					size_t cosntIndex = name.find(constStr);
					if (cosntIndex != name.npos)
						isConst = true;
				}
			}

			if constexpr (std::is_lvalue_reference_v<T>)
				isLref = true;

			if constexpr (std::is_rvalue_reference_v<T>)
				isRref = true;

			using valueType = RemoveAll_T<T>;

			if constexpr (std::is_void_v<valueType>)
				jDataEnum = J_PARAMETER_TYPE::Void;
			else if constexpr (std::is_enum_v<valueType>)
				jDataEnum = J_PARAMETER_TYPE::Enum;
			else if constexpr (std::is_class_v<valueType>)
			{
				if constexpr (std::is_same_v<valueType, std::string>)
					jDataEnum = J_PARAMETER_TYPE::String;
				else
					jDataEnum = J_PARAMETER_TYPE::Class;
			}
			else if constexpr (std::is_floating_point_v<valueType>)
				jDataEnum = J_PARAMETER_TYPE::Float;
			else if constexpr (std::is_integral_v<valueType>)
			{
				if constexpr (std::is_same_v<valueType, bool>)
					jDataEnum = J_PARAMETER_TYPE::Bool;
				else
					jDataEnum = J_PARAMETER_TYPE::Int;
			}

			return JParameterHint(name, isConst, isPtr, isArray, isLref, isRref, dimension, jDataEnum, arrLastDimCount);
		}

		template<typename ...Param, size_t ...Is>
		static void CreateParameterHint(std::index_sequence<Is...>, std::vector<JParameterHint>& hintVec, const std::vector<std::string>& rawPramVec)
		{
			std::string sum;
			((hintVec.emplace_back(CreateParameterHint<Param>(rawPramVec[Is]))), ...);
		}

		template<typename ...Param, size_t ...Is>
		bool IsConvertible(std::vector<JParameterHint>& hint, std::index_sequence<Is...>)
		{
			return (hint[Is].IsConvertible<Param>() && ...);
		}
	}
}
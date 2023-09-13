#pragma once 
#include<Windows.h>
#include<string>
#include<sstream> 
#include<DirectXMath.h>
#include"../../Core/Utility/JTypeTraitUtility.h"
 
namespace fbxsdk
{ 
	class FbxMatrix;
	class FbxAMatrix;
	class FbxVector4;
	class FbxQuaternion;
}
namespace JinEngine
{
	class JTypeToString
	{
	public:
		static std::string ConvertValue(const fbxsdk::FbxMatrix& matrix, const std::string& guide = "FbxMatrix")noexcept;
		static std::string ConvertValue(const fbxsdk::FbxAMatrix& matrix, const std::string& guide = "FbxAMatrix", const bool decomposeVector = false)noexcept;
		static std::string ConvertValue(const fbxsdk::FbxVector4& vector, const std::string& guide = "FbxVector4")noexcept;
		static std::string ConvertValue(const fbxsdk::FbxQuaternion& quaternion, const std::string& guide = "FbxQuaternion")noexcept;
		static std::string ConvertValue(const DirectX::XMFLOAT4X4& matrix, const std::string& guide = "XMFLOAT4X4", const bool decomposeVector = false)noexcept;
		static std::string ConvertValue(const DirectX::XMMATRIX& matrix, const std::string& guide = "XMMATRIX", const bool decomposeVector = false)noexcept;
		static std::string ConvertValue(const DirectX::XMFLOAT2& vector, const std::string& guide = "XMFLOAT2")noexcept;
		static std::string ConvertValue(const DirectX::XMFLOAT3& vector, const std::string& guide = "XMFLOAT3")noexcept;
		static std::string ConvertValue(const DirectX::XMFLOAT4& vector, const std::string& guide = "XMFLOAT4")noexcept;
		static std::string ConvertValue(const DirectX::XMVECTOR& vector, const std::string& guide = "XMVECTOR")noexcept;
		static std::string LastErrorConvert(HWND hwnd);
		static std::string HResultConvert(HRESULT hr);
	public:
		template<typename T>
		static std::string PrintType()
		{
			std::stringstream stream;
			stream << "PrintType: \n";
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>>)
				stream << "const ";

			stream << typeid(T).name();

			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				stream << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				stream << "&";

			stream << "\n";
			return stream.str();
		}
		template<typename T>
		static std::string PrintTypeAValue(T t)
		{
			std::stringstream stream;
			stream << "PrintTypeAValue: \n";
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>>)
				stream << "const ";

			stream << typeid(T).name();
			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				stream << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				stream << "&"; 
			stream << t << "\n";
			return stream.str();
		}
		template<typename T>
		static std::string PrintTypeUValue(T&& t)
		{
			std::stringstream stream;
			stream << "PrintTypeUValue: \n";
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>>)
				stream << "const ";

			stream << typeid(T).name();

			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				stream << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<T>>>)
				stream << "&";

			stream << "\n";
			stream << "decl\n";
			if constexpr (std::is_const_v<std::remove_reference_t<std::remove_pointer_t<std::remove_all_extents_t<decltype(t)>>>>)
				stream << "const ";

			stream << typeid(decltype(t)).name();
			if constexpr (std::is_rvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<decltype(t)>>>)
				stream << "&&";
			if constexpr (std::is_lvalue_reference_v<std::remove_pointer_t<std::remove_all_extents_t<decltype(t)>>>)
				stream << "&";

			stream << "\n";

			if constexpr (std::is_array_v<std::remove_reference_t<decltype(t)>>)
			{
				using ValueType = Core::RemoveAll_T<decltype(t)>;
				size_t eleSize = sizeof(t) / sizeof(ValueType);
				stream << "Type: " << typeid(ValueType).name() << "\n";
				stream << "EleSize: " << eleSize << "\n";
				stream << "Value: \n";
				for (int i = 0; i < eleSize; ++i)
					stream << t[i] << "\n";
			}
			else  if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(t)>>)
			{
				using ValueType = Core::RemoveAll_T<decltype(t)>;
				stream << "Type: " << typeid(ValueType).name() << "\n";
				stream << "Value: \n";
				if constexpr (std::is_integral_v<ValueType>)
					stream << *t <<"\n";
			}
			stream << "\n";
			return stream.str();
		}
		template<typename ...Param>
		static std::string PrintParams()
		{
			std::stringstream stream; 
			stream << "Print Param \n";
			((stream << PrintType<Param>()), ...);   
			return stream.str();
		}
		template<typename ...Param>
		static std::string PrintParamValues(Param&&... var)
		{
			std::stringstream stream; 
			stream << "Print Param values" << std::endl;
			((stream << PrintTypeUValue(std::forward<Param>(var))), ...);
			return stream.str();
		}
		template<typename ...Param>
		static std::string PrintTupleParamType(std::tuple<Param...>& tuple)
		{
			std::stringstream stream; 
			stream << "Print Tuple Type Param" << std::endl; 
			((stream << PrintType<Param>()), ...); 
			return stream.str();
		}
		template<size_t ...Is, typename Tulpe>
		static std::string PrintTupleParamValueType(std::index_sequence<Is...>, Tulpe& t)
		{
			std::stringstream stream; 
			stream << "Print Tuple Value Type Param" << std::endl;
			((stream << PrintTypeAValue(std::get<Is>(std::move(t)))), ...);
			return stream.str();
		}
	};
}
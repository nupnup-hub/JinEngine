#pragma once 
#include<type_traits>
#include<fstream> 
#include<string> 
#include<Windows.h>
#include <DirectXMath.h>
#include"fbxsdk/core/math/fbxmatrix.h"
#include"fbxsdk/core/math/fbxvector4.h"
#include"fbxsdk/core/math/fbxdualquaternion.h"
#include"fbxsdk/core/math/fbxaffinematrix.h"
#include"../Core/JDataType.h"

namespace JinEngine
{
	class JGameObject;
	namespace Core
	{
		class JDebugTextOut
		{
		private:
			static std::wofstream stream;
			static int count;

		public:
			static bool OpenStream(const std::wstring& path, int config, int count);
			static bool CloseStream();
			static bool IsOpenedStream();

			static void PrintStr(const std::string& str)noexcept;
			static void PrintWstr(const std::wstring& str)noexcept;
			static void PrintMatrix(const fbxsdk::FbxAMatrix& matrix, const std::wstring& guide)noexcept;
			static void PrintMatrix(const fbxsdk::FbxMatrix& matrix, const std::wstring& guide)noexcept;
			static void PrintMatrix(const DirectX::XMFLOAT4X4& matrix, const std::wstring& guide)noexcept;
			static void PrintMatrix(const DirectX::XMMATRIX matrix, const std::wstring& guide)noexcept;
			static void PrintMatrixAndVector(const  fbxsdk::FbxAMatrix& matrix, const std::wstring& guide)noexcept;
			static void PrintMatrixAndVector(const  DirectX::XMFLOAT4X4& matrix, const std::wstring& guide)noexcept;
			static void PrintMatrixAndVector(const  DirectX::XMMATRIX matrix, const std::wstring& guide)noexcept;
			static void PrintMatrixTQS(const  DirectX::XMFLOAT4X4& matrix, const std::wstring& guide)noexcept;
			static void PrintMatrixTQS(const  DirectX::XMMATRIX matrix, const std::wstring& guide)noexcept;
			static void PrintVector(const fbxsdk::FbxVector4& vector, const std::wstring& guide)noexcept;
			static void PrintVector(const DirectX::XMFLOAT3& vector, const std::wstring& guide)noexcept;
			static void PrintVector(const DirectX::XMFLOAT4& vector, const std::wstring& guide)noexcept;
			static void PrintVector(const DirectX::XMVECTOR& vector, const std::wstring& guide)noexcept;
			static void PrintQuaternion(const fbxsdk::FbxQuaternion& quaternion, const std::wstring& guide)noexcept;
			static void PrintQuaternion(const DirectX::XMFLOAT4& vector, const std::wstring& guide)noexcept;
			static void PrintQuaternion(const DirectX::XMVECTOR& vector, const std::wstring& guide)noexcept;
			template<typename T, typename C = std::enable_if_t<std::is_integral<T>::value || std::is_unsigned<T>::value>>
			static void PrintIntData(const T data, const std::wstring& guide)noexcept;
			template<typename T, typename C = std::enable_if_t<std::is_floating_point<T>::value>>
			static void PrintFloatData(const T data, const std::wstring& guide)noexcept;
			static void PrintSpace(const uint count)noexcept;
			static void PrintEnter(const uint count)noexcept;
			static void PrintLastError(HWND hwnd);
			static void PrintHresult(HRESULT hr);
			static void PrintGameObjectTree(JGameObject* root);
		private:
			static void PrintGameObjectTreeLoop(JGameObject* obj, uint level);
		};
	} 
}
#include"JDebugTextOut.hpp"
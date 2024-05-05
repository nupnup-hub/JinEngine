#include"JTypeToString.h"
#include"fbxsdk/core/math/fbxmatrix.h"
#include"fbxsdk/core/math/fbxvector4.h"
#include"fbxsdk/core/math/fbxdualquaternion.h"
#include"fbxsdk/core/math/fbxaffinematrix.h" 
#include<type_traits> 
#include<Windows.h>
#include <DirectXMath.h>

using namespace DirectX;
using namespace fbxsdk;
namespace JinEngine
{
	std::string JTypeToString::ConvertValue(const FbxMatrix& matrix, const std::string& guide)noexcept
	{
		std::stringstream stream;
		stream << guide << '\n';
		stream << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << " " << matrix[3][0] << '\n';
		stream << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << " " << matrix[3][1] << '\n';
		stream << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << " " << matrix[3][2] << '\n';
		stream << matrix[0][3] << " " << matrix[1][3] << " " << matrix[2][3] << " " << matrix[3][3] << '\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const FbxAMatrix& matrix, const std::string& guide, const bool decomposeVector)noexcept
	{
		std::stringstream stream;
		stream << guide << '\n';
		stream << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << " " << matrix[3][0] << '\n';
		stream << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << " " << matrix[3][1] << '\n';
		stream << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << " " << matrix[3][2] << '\n';
		stream << matrix[0][3] << " " << matrix[1][3] << " " << matrix[2][3] << " " << matrix[3][3] << '\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const FbxVector4& vector, const std::string& guide)noexcept
	{
		std::stringstream stream;
		stream << guide << " " << vector[0] << " " << vector[1] << " " << vector[2] << " " << vector[3] << '\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const FbxQuaternion& quaternion, const std::string& guide)noexcept
	{
		std::stringstream stream;
		stream << guide << " " << quaternion[0] << " " << quaternion[1] << " " << quaternion[2] << " " << quaternion[3] << '\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const XMFLOAT4X4& matrix, const std::string& guide, const bool decomposeVector)noexcept
	{
		if (decomposeVector)
			return ConvertValue(XMLoadFloat4x4(&matrix), guide, decomposeVector);
		else
		{
			std::stringstream stream;
			stream << guide << '\n';
			stream << matrix._11 << " " << matrix._12 << " "
				<< matrix._13 << " " << matrix._14 << '\n';
			stream << matrix._21 << " " << matrix._22 << " "
				<< matrix._23 << " " << matrix._24 << '\n';
			stream << matrix._31 << " " << matrix._32 << " "
				<< matrix._33 << " " << matrix._34 << '\n';
			stream << matrix._41 << " " << matrix._42 << " "
				<< matrix._43 << " " << matrix._44 << '\n';
			return stream.str();
		}
	}
	std::string JTypeToString::ConvertValue(const XMMATRIX& matrix, const std::string& guide, const bool decomposeVector)noexcept
	{
		if (decomposeVector)
		{
			std::stringstream stream;
			XMVECTOR s;
			XMVECTOR q;
			XMVECTOR t;
			XMMatrixDecompose(&s, &q, &t, matrix);
			stream << ConvertValue(t, "T") << '\n';
			stream << ConvertValue(q, "Q") << '\n';
			stream << ConvertValue(s, "S") << '\n';
			return stream.str();
		}
		else
		{
			XMFLOAT4X4 m;
			XMStoreFloat4x4(&m, matrix);
			return ConvertValue(m, guide, decomposeVector);
		}
	}
	std::string JTypeToString::ConvertValue(const DirectX::XMFLOAT2& vector, const std::string& guide)noexcept
	{
		std::stringstream stream;
		stream << guide << " " << vector.x << " " << vector.y <<'\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const XMFLOAT3& vector, const std::string& guide)noexcept
	{
		std::stringstream stream;
		stream << guide << " " << vector.x << " " << vector.y << " " << vector.z << '\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const XMFLOAT4& vector, const std::string& guide)noexcept
	{
		std::stringstream stream;
		stream << guide << " " << vector.x << " " << vector.y << " " << vector.z << " " << vector.w << '\n';
		return stream.str();
	}
	std::string JTypeToString::ConvertValue(const XMVECTOR& vector, const std::string& guide)noexcept
	{
		XMFLOAT4 f4;
		XMStoreFloat4(&f4, vector);
		return ConvertValue(f4, guide);
	}
	std::string JTypeToString::LastErrorConvert(HWND hwnd)
	{
		std::stringstream stream; 
		stream << L"LastError" << '\n';

		DWORD dwError = GetDlgItemInt(hwnd, 1000, NULL, FALSE);

		HLOCAL hlocal = NULL;   // Buffer that gets the error message wstring

		// Use the default system locale since we look for Windows messages.
		// Note: this MAKELANGID combination has 0 as value
		DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

		// Get the error code's textual description
		BOOL fOk = FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL, dwError, systemLocale,
			(PTSTR)&hlocal, 0, NULL);

		if (!fOk) {
			// Is it a network-related error?
			HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL,
				DONT_RESOLVE_DLL_REFERENCES);

			if (hDll != NULL) {
				fOk = FormatMessage(
					FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
					FORMAT_MESSAGE_ALLOCATE_BUFFER,
					hDll, dwError, systemLocale,
					(PTSTR)&hlocal, 0, NULL);
				FreeLibrary(hDll);
			}
		}

		if (fOk && (hlocal != NULL))
		{
			stream << (PCTSTR)LocalLock(hlocal) << '\n';
			LocalFree(hlocal);
		}
		else
			stream << L"No text found for this error number." << '\n';
		return stream.str();
	}
	std::string JTypeToString::HResultConvert(HRESULT hr)
	{
		std::stringstream stream; 
		std::string message00 = std::system_category().message(hr);
		stream << L"Hresult" << '\n';
		stream << message00 << '\n';
		return stream.str();
	}
}
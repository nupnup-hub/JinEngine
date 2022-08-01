#include"JDebugTextOut.h"
#include"../Utility/JMathHelper.h" 
#include"../Utility/JCommonUtility.h"
#include"../Object/GameObject/JGameObject.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{

		std::wofstream JDebugTextOut::stream;
		int JDebugTextOut::count = 0;
		bool JDebugTextOut::OpenStream(const std::wstring& path, int config, int count)
		{
			if (stream.is_open())
				stream.close();

			if (JDebugTextOut::count == 0)
				JDebugTextOut::count = count;

			if (JDebugTextOut::count > 0)
				stream.open(path, config);
			return true;
		}
		bool JDebugTextOut::CloseStream()
		{
			if (stream.is_open())
				stream.close();

			--JDebugTextOut::count;
			if (JDebugTextOut::count == 0)
				JDebugTextOut::count = -1;
			return true;
		}
		bool JDebugTextOut::IsOpenedStream()
		{
			return stream.is_open();
		}
		void JDebugTextOut::PrintStr(const std::string& str)noexcept
		{
			stream << JCommonUtility::U8StringToWstring(str) << '\n';
		}
		void JDebugTextOut::PrintWstr(const std::wstring& str)noexcept
		{
			stream << str << '\n';
		}
		void JDebugTextOut::PrintMatrix(const fbxsdk::FbxAMatrix& matrix, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << " " << matrix[3][0] << '\n';
			stream << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << " " << matrix[3][1] << '\n';
			stream << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << " " << matrix[3][2] << '\n';
			stream << matrix[0][3] << " " << matrix[1][3] << " " << matrix[2][3] << " " << matrix[3][3] << '\n';
			stream << '\n';
		}
		void JDebugTextOut::PrintMatrix(const fbxsdk::FbxMatrix& matrix, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << " " << matrix[3][0] << '\n';
			stream << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << " " << matrix[3][1] << '\n';
			stream << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << " " << matrix[3][2] << '\n';
			stream << matrix[0][3] << " " << matrix[1][3] << " " << matrix[2][3] << " " << matrix[3][3] << '\n';
			stream << '\n';
		}
		void JDebugTextOut::PrintMatrix(const DirectX::XMFLOAT4X4& matrix, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << matrix._11 << " " << matrix._12 << " "
				<< matrix._13 << " " << matrix._14 << '\n';
			stream << matrix._21 << " " << matrix._22 << " "
				<< matrix._23 << " " << matrix._24 << '\n';
			stream << matrix._31 << " " << matrix._32 << " "
				<< matrix._33 << " " << matrix._34 << '\n';
			stream << matrix._41 << " " << matrix._42 << " "
				<< matrix._43 << " " << matrix._44 << '\n';
			stream << '\n';
		}
		void JDebugTextOut::PrintMatrix(const DirectX::XMMATRIX matrix, const std::wstring& guide)noexcept
		{
			DirectX::XMFLOAT4X4 fMatrix;
			DirectX::XMStoreFloat4x4(&fMatrix, matrix);
			PrintMatrix(fMatrix, guide);
		}
		void JDebugTextOut::PrintMatrixAndVector(const fbxsdk::FbxAMatrix& matrix, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << " " << matrix[3][0] << '\n';
			stream << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << " " << matrix[3][1] << '\n';
			stream << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << " " << matrix[3][2] << '\n';
			stream << matrix[0][3] << " " << matrix[1][3] << " " << matrix[2][3] << " " << matrix[3][3] << '\n';

			PrintVector(matrix.GetS(), L"Scale: ");
			PrintQuaternion(matrix.GetQ(), L"Quaternion: ");
			PrintVector(matrix.GetT(), L"Translation: ");
			stream << '\n';
		}

		void JDebugTextOut::PrintMatrixAndVector(const  DirectX::XMFLOAT4X4& matrix, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << L"Matrix" << '\n';
			stream << matrix._11 << " " << matrix._12 << " "
				<< matrix._13 << " " << matrix._14 << '\n';
			stream << matrix._21 << " " << matrix._22 << " "
				<< matrix._23 << " " << matrix._24 << '\n';
			stream << matrix._31 << " " << matrix._32 << " "
				<< matrix._33 << " " << matrix._34 << '\n';
			stream << matrix._41 << " " << matrix._42 << " "
				<< matrix._43 << " " << matrix._44 << '\n';

			DirectX::XMMATRIX xm = XMLoadFloat4x4(&matrix);
			DirectX::XMVECTOR s;
			DirectX::XMVECTOR q;
			DirectX::XMVECTOR t;
			DirectX::XMMatrixDecompose(&s, &q, &t, xm);
			PrintVector(s, L"Scale: ");
			PrintQuaternion(q, L"Quaternion: ");
			PrintVector(t, L"Translation: ");
			stream << '\n';
		}
		void JDebugTextOut::PrintMatrixAndVector(const DirectX::XMMATRIX matrix, const std::wstring& guide)noexcept
		{
			DirectX::XMFLOAT4X4 fMatrix;
			DirectX::XMStoreFloat4x4(&fMatrix, matrix);
			PrintMatrixAndVector(fMatrix, guide);
		}
		void JDebugTextOut::PrintMatrixTQS(const DirectX::XMFLOAT4X4& matrix, const std::wstring& guide)noexcept
		{
			DirectX::XMMATRIX xm = XMLoadFloat4x4(&matrix);
			PrintMatrixTQS(xm, guide);
		}
		void JDebugTextOut::PrintMatrixTQS(const  DirectX::XMMATRIX matrix, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			DirectX::XMVECTOR s;
			DirectX::XMVECTOR q;
			DirectX::XMVECTOR t;
			DirectX::XMMatrixDecompose(&s, &q, &t, matrix);
			PrintVector(s, L"Scale: ");
			PrintQuaternion(q, L"Quaternion: ");
			PrintVector(t, L"Translation: ");
			stream << '\n';
		}
		void JDebugTextOut::PrintVector(const fbxsdk::FbxVector4& vector, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << vector[0] << " " << vector[1] << " " << vector[2] << " " << vector[3] << '\n';
		}
		void JDebugTextOut::PrintVector(const DirectX::XMFLOAT3& vector, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << vector.x << " " << vector.y << " " << vector.z << '\n';
		}
		void JDebugTextOut::PrintVector(const  DirectX::XMFLOAT4& vector, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << vector.x << " " << vector.y << " " << vector.z << " " << vector.w << '\n';
		}
		void JDebugTextOut::PrintVector(const  DirectX::XMVECTOR& vector, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			DirectX::XMFLOAT4 fV;
			DirectX::XMStoreFloat4(&fV, vector);
			stream << guide << '\n';
			stream << fV.x << " " << fV.y << " " << fV.z << " " << fV.w << '\n';
		}
		void JDebugTextOut::PrintQuaternion(const fbxsdk::FbxQuaternion& quaternion, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << quaternion[0] << " " << quaternion[1] << " " << quaternion[2] << " " << quaternion[3] << '\n';

			DirectX::XMFLOAT4 qXm{ (float)quaternion[0], (float)quaternion[1], (float)quaternion[2],(float)quaternion[3] };
			DirectX::XMFLOAT3 eulerAngle = JMathHelper::ToEulerAngle(qXm);
			stream << L"Euler" << '\n';
			stream << eulerAngle.x << " " << eulerAngle.y << " " << eulerAngle.z << '\n';
		}
		void JDebugTextOut::PrintQuaternion(const DirectX::XMFLOAT4& quaternion, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			stream << guide << '\n';
			stream << quaternion.x << " " << quaternion.y << " " << quaternion.z << " " << quaternion.w << '\n';

			DirectX::XMFLOAT4 qXm{ quaternion.x, quaternion.y, quaternion.z,quaternion.w };
			DirectX::XMFLOAT3 eulerAngle = JMathHelper::ToEulerAngle(qXm);

			stream << L"Euler" << '\n';
			stream << eulerAngle.x << " " << eulerAngle.y << " " << eulerAngle.z << '\n';
		}
		void JDebugTextOut::PrintQuaternion(const DirectX::XMVECTOR& vector, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;

			DirectX::XMFLOAT4 fV;
			DirectX::XMStoreFloat4(&fV, vector);

			stream << guide << '\n';
			stream << fV.x << " " << fV.y << " " << fV.z << " " << fV.w << '\n';

			DirectX::XMFLOAT3 eulerAngle = JMathHelper::ToEulerAngle(fV);
			stream << L"Euler" << '\n';
			stream << eulerAngle.x << " " << eulerAngle.y << " " << eulerAngle.z << '\n';
		}
		void JDebugTextOut::PrintSpace(const uint count)noexcept
		{
			if (!stream.is_open())
				return;

			for (uint i = 0; i < count; ++i)
				stream << " ";
		}
		void JDebugTextOut::PrintEnter(const uint count)noexcept
		{
			if (!stream.is_open())
				return;

			for (uint i = 0; i < count; ++i)
				stream << '\n';
		}
		void JDebugTextOut::PrintLastError(HWND hwnd)
		{
			if (!stream.is_open())
				return;

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
		}
		void JDebugTextOut::PrintHresult(HRESULT hr)
		{
			if (!stream.is_open())
				return;

			std::string message00 = std::system_category().message(hr);
			stream << L"Hresult" << '\n';
			stream << JCommonUtility::StringToWstring(message00) << '\n';
		}
		void JDebugTextOut::PrintGameObjectTree(JGameObject* root)
		{
			if (!stream.is_open())
				return;

			PrintGameObjectTreeLoop(root, 0);
		}
		void JDebugTextOut::PrintGameObjectTreeLoop(JGameObject* obj, uint level)
		{
			PrintSpace(level);
			stream << obj->GetWName() << " HasR: " << obj->HasRenderItem() << '\n';

			const uint childrenCount = obj->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
				PrintGameObjectTreeLoop(obj->GetChild(i), level + 1);
		}
	}
}
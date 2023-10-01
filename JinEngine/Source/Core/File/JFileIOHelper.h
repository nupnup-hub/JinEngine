#pragma once
#include"../../Core/File/JFileIOResult.h"
#include"../../Core/FSM/JFSMobjectType.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/Math/JVector.h"  
#include"../../Core/Reflection/JTypeBase.h"
#include<DirectXMath.h>
#include<fstream>

namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
		class JFSMinterface;
	}
	class JFileIOHelper
	{
	public:
		using TryAgainLoadObjIfFailPtr = JUserPtr<Core::JIdentifier>(*)(Core::JTypeInfo*, size_t);
		using TryAgainLoadHintIfFailPtr = Core::JTypeInstanceSearchHint(*)(Core::JTypeInfo*, size_t);
	public:
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT StoreAtomicData(std::wofstream& stream, const std::wstring& guide, T value, const bool useChangeLine = true)
		{
			if (!stream.is_open())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			if (useChangeLine)
				stream << guide << " " << value << '\n';
			else
				stream << guide << " " << value << ' ';
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT LoadAtomicData(std::wifstream& stream, T& value)
		{
			if (!stream.is_open())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			std::wstring guide;
			stream >> guide >> value;
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT StoreEnumData(std::wofstream& stream, const std::wstring& guide, T value, const bool useChangeLine = true)
		{
			if (!stream.is_open())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			if (useChangeLine)
				stream << guide << " " << (int)value << '\n';
			else
				stream << guide << " " << (int)value << ' ';
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT LoadEnumData(std::wifstream& stream, T& eValue)
		{
			if (!stream.is_open())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			std::wstring guide;
			int value;
			stream >> guide >> value;
			eValue = (T)value;
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		template<typename T>
		static Core::J_FILE_IO_RESULT StoreVector2(std::wofstream& stream, const std::wstring& guide, const JVector2<T>& value)
		{
			if constexpr (JVector2<T>::isValidValue)
			{
				if (!stream.is_open())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				stream << guide << " " << value.x << " " << value.y << '\n';
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT StoreVector3(std::wofstream& stream, const std::wstring& guide, const JVector3<T>& value)
		{
			if constexpr (JVector3<T>::isValidValue)
			{
				if (!stream.is_open())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				stream << guide << " " << value.x << " " << value.y << " " << value.z << '\n';
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT StoreVector4(std::wofstream& stream, const std::wstring& guide, const JVector4<T>& value)
		{
			if constexpr (JVector4<T>::isValidValue)
			{
				if (!stream.is_open())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				stream << guide << " " << value.x << " " << value.y << " " << value.z << " " << value.w << '\n';
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT LoadVector2(std::wifstream& stream, JVector2<T>& v2)
		{
			if constexpr (JVector2<T>::isValidValue)
			{
				if (!stream.is_open() || stream.eof())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				std::wstring guide;
				stream >> guide >> v2.x >> v2.y;
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT LoadVector3(std::wifstream& stream, JVector3<T>& v3)
		{
			if constexpr (JVector3<T>::isValidValue)
			{
				if (!stream.is_open() || stream.eof())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				std::wstring guide;
				stream >> guide >> v3.x >> v3.y >> v3.z;
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA; 
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT LoadVector4(std::wifstream& stream, JVector4<T>& v4)
		{
			if constexpr (JVector4<T>::isValidValue)
			{
				if (!stream.is_open() || stream.eof())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				std::wstring guide;
				stream >> guide >> v4.x >> v4.y >> v4.z >> v4.w;
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
	public:
		static Core::J_FILE_IO_RESULT StoreMatrix4x4(std::wofstream& stream, const std::wstring& guide, const JMatrix4x4& m);
		static Core::J_FILE_IO_RESULT LoadMatrix4x4(std::wifstream& stream, JMatrix4x4& m);
	public:
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT StoreAtomicDataVec(std::wofstream& stream, const std::wstring& guide, const std::vector<T>& vec, int spaceoffset)
		{
			if (!stream.is_open())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			stream << guide << '\n';
			stream << L"SpaceOffset: " << spaceoffset << '\n';
			stream << L"VecCount: " << vec.size() << '\n';

			int vecCount = (int)vec.size();
			for (int i = 0; i < vecCount; ++i)
			{
				if (i % spaceoffset == 0 && i != 0)
					stream << '\n';
				stream << vec[i] << " ";
			}
			stream << '\n';
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT LoadAtomicDataVec(std::wifstream& stream, std::vector<T>& vec)
		{
			if (!stream.is_open() || stream.eof())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			std::wstring guide;
			int spaceOffset = 0;
			int vecCount = 0;
			stream >> guide;
			stream >> guide >> spaceOffset;
			stream >> guide >> vecCount;

			vec.resize(vecCount);
			for (int i = 0; i < vecCount; ++i)
				stream >> vec[i];

			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		static Core::J_FILE_IO_RESULT StoreJString(std::wofstream& stream, const std::wstring& guide, const std::wstring& str, const bool useChangeLine = true);
		static Core::J_FILE_IO_RESULT LoadJString(std::wifstream& stream, std::wstring& str);
	public:
		static Core::J_FILE_IO_RESULT StoreXMFloat2(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT2& xm2);
		static Core::J_FILE_IO_RESULT StoreXMFloat3(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT3& xm3);
		static Core::J_FILE_IO_RESULT StoreXMFloat4(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT4& xm4);
		static Core::J_FILE_IO_RESULT StoreXMFloat4x4(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT4X4& xm4x4);
		static Core::J_FILE_IO_RESULT LoadXMFloat2(std::wifstream& stream, DirectX::XMFLOAT2& xm2);
		static Core::J_FILE_IO_RESULT LoadXMFloat3(std::wifstream& stream, DirectX::XMFLOAT3& xm3);
		static Core::J_FILE_IO_RESULT LoadXMFloat4(std::wifstream& stream, DirectX::XMFLOAT4& xm4);
		static Core::J_FILE_IO_RESULT LoadXMFloat4x4(std::wifstream& stream, DirectX::XMFLOAT4X4& xm4x4);
	public:
		static Core::J_FILE_IO_RESULT StoreIden(std::wofstream& stream, Core::JIdentifier* obj);
		static Core::J_FILE_IO_RESULT LoadIden(std::wifstream& stream, _Out_ size_t& oGuid);
		static Core::J_FILE_IO_RESULT LoadIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid);
	public:
		static Core::J_FILE_IO_RESULT StoreFsmIden(std::wofstream& stream, Core::JFSMinterface* obj);
		static Core::J_FILE_IO_RESULT LoadFsmIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ Core::J_FSM_OBJECT_TYPE& oType);
	public:
		static Core::J_FILE_IO_RESULT StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden);
		static Core::J_FILE_IO_RESULT StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden, const std::wstring& guiSymbol);
		static Core::J_FILE_IO_RESULT StoreHasInstanceHint(std::wofstream& stream, const Core::JTypeInstanceSearchHint& hint);
		static Core::J_FILE_IO_RESULT StoreHasInstanceHint(std::wofstream& stream, const Core::JTypeInstanceSearchHint& hint, const std::wstring& guiSymbol);
		//Warrning
		//Loading중 다른 오브젝트를 Load할 수 있으므로
		//Stream에러 발생 할 수있음
		//LoadHasObjectHint는 Hint만 전달하므로 Stream에러 발생에서 안전함
		static JUserPtr<Core::JIdentifier> LoadHasIden(std::wifstream& stream, TryAgainLoadObjIfFailPtr tryAgainPtr = nullptr);
		template<typename T>
		static JUserPtr<T> LoadHasIden(std::wifstream& stream, TryAgainLoadObjIfFailPtr tryAgainPtr = nullptr)
		{
			return JUserPtr<T>::ConvertChild(LoadHasIden(stream, tryAgainPtr));
		}
		static Core::JTypeInstanceSearchHint LoadHasIdenHint(std::wifstream& stream, TryAgainLoadHintIfFailPtr tryAgainPtr = nullptr);
	public:
		//skip file using getline until symbol
		//if fail stream is close
		static bool SkipLine(std::wifstream& stream, const std::wstring& symbol);
		//skip file using stream until symbol
		//if fail stream is close
		static bool SkipSentence(std::wifstream& stream, const std::wstring& symbol);
	public:
		static bool InputSpace(std::wofstream& stream, int spaceCount);
	public:
		static Core::J_FILE_IO_RESULT CopyFile(const std::wstring& from, const std::wstring& to);
		//write to fixed symbol on toFile when end of  fromFile
		static Core::J_FILE_IO_RESULT CombineFile(const std::vector<std::wstring> from, const std::wstring& to);
		//read to fixed symbol on fromFile when end of toFile
		static Core::J_FILE_IO_RESULT DevideFile(const std::wstring& from, const std::vector<std::wstring> to);
		static Core::J_FILE_IO_RESULT DestroyFile(const std::wstring& path);
		static Core::J_FILE_IO_RESULT DestroyDirectory(const std::wstring& path);
	public:
		/**
		* @return return empty if invalid path
		*/
		static std::string FileToString(const std::string& path);
		/**
		* @return return empty if invalid path
		*/
		static std::wstring FileToWString(const std::wstring& path);
	};
}
/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../../Core/File/JFileIOResult.h"
#include"../../Core/File/JJSon.h"
#include"../../Core/FSM/JFSMobjectType.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/Math/JVector.h"  
#include"../../Core/Reflection/JTypeBase.h"
#include<DirectXMath.h>
#include<fstream> 

#define FILE_ASSERTION(result) if(result != Core::J_FILE_IO_RESULT::SUCCESS)  MessageBoxA(0, ("Fali\n" + std::string(__FILE__) + " " + std::to_string(__LINE__)).c_str(), 0, 0);
#define FILE_TOOL_ASSERTION(res) if(!res) MessageBoxA(0, ("FileToolError\n" + std::string(__FILE__) + " " + std::to_string(__LINE__)).c_str(), 0, 0);
namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
		class JFSMinterface;
	}

	class JFileIOHelper;
	class JFileIOTool
	{
	public:
		enum class TYPE
		{
			INPUT_STREAM,
			OUTPUT_STREAM,
			JSON
		};
		enum BEGIN_OPTION
		{
			BEGIN_OPTION_NONE = 0,
			BEGIN_OPTION_JSON_TRY_LOAD_DATA = 1 << 0,
			BEGIN_OPTION_JSON_SET_CONTENTS_ARRAY_OWNER = 1 << 1
		};
		enum CLOSE_OPTION
		{
			CLOSE_OPTION_NONE = 0,
			CLOSE_OPTION_JSON_STORE_DATA = 1 << 0,	//for json 
		};
	public:
		struct NextCurrentHint
		{
		public: 
			std::string key;
			int index = invalidIndex;
		public:
			NextCurrentHint(const std::string& key);
			NextCurrentHint(const int index);
		};
	private:
		friend class JFileIOHelper;
	private:
		std::unique_ptr<std::ifstream> istream;
		std::unique_ptr<std::ofstream> ostream;
	private:
		std::unique_ptr<Core::JJSon> json;
		std::string guide;
	private:
		std::unique_ptr<NextCurrentHint> nextCurHint;
	public:
		NextCurrentHint* GetNextCurrentHint()const noexcept;
		uint GetCurrentMemberCount()const noexcept;
	public:
		bool CanLoad()const noexcept;
		bool CanStore()const noexcept;
		bool CanUseInputStream()const noexcept;
		bool CanUseOutputStream()const noexcept;
		bool CanUseJSon()const noexcept;
	public:
		bool Begin(const std::string& path, const TYPE toolType, const BEGIN_OPTION option = BEGIN_OPTION_NONE);
		bool Begin(const std::wstring& path, const TYPE toolType, const BEGIN_OPTION option = BEGIN_OPTION_NONE);
	private:
		bool BeginInputStream(const std::string& path);
		bool BeginOutputStream(const std::string& path);
		bool BeginJSon(const std::string& path, const bool isContentsMemberArray = false);
		bool BeginAndTryLoadJSon(const std::string& path);
	public:
		void Close(const CLOSE_OPTION option = CLOSE_OPTION_NONE);
	private:
		void CloseInputStream();
		void CloseOutputStream();
		void CloseJSon(const bool allowStoreJSon = false);
	public:
		void StoreJSon();
	public:
		/*
		* @parameter push array container if jason or output key
		*/
		bool PushArrayOwner(const std::string& key = "");
		/*
		* @parameter push array element if jason
		*/
		bool PushArrayMember();
		/*
		* @parameter push map element if jason  or output key
		*/
		bool PushMapMember(const std::string& key);
		bool PushMember(const std::string& key = "");
		bool PushExistStack(const std::string& key = "");
		bool PopStack();
	public:
		NextCurrentHint* CreateNextCurrentHint(std::unique_ptr<NextCurrentHint>&& hint);
		void ClearNextCurrentHint();
	};
	class JFileIOHelper
	{
	public:
		using TryAgainLoadObjIfFailPtr = JUserPtr<Core::JIdentifier>(*)(Core::JTypeInfo*, size_t);
		using TryAgainLoadHintIfFailPtr = Core::JTypeInstanceSearchHint(*)(Core::JTypeInfo*, size_t);
	public:
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT StoreAtomicData(JFileIOTool& tool, T value, const std::string& guide)
		{
			if(!tool.CanStore())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			if (tool.CanUseJSon())
				Core::JJSon::StoreData(*tool.json, value, guide);
			else
				(*tool.ostream) << guide << " " << value << '\n';
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT LoadAtomicData(JFileIOTool& tool, T& value, const std::string& key = "")
		{
			if(!tool.CanLoad())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			if (tool.CanUseJSon())
			{
				if (!Core::JJSon::LoadData(*tool.json, value, key))
					return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
			}
			else
			{
				std::string guide;
				(*tool.istream) >> guide >> value;
			}
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT StoreEnumData(JFileIOTool& tool, T value, const std::string& guide)
		{
			if(!tool.CanStore())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			if (tool.CanUseJSon())
				Core::JJSon::StoreData(*tool.json, value, guide);
			else
				(*tool.ostream) << guide << " " << (int)value << '\n';
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT LoadEnumData(JFileIOTool& tool, T& eValue, const std::string& key = "")
		{
			if(!tool.CanLoad())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			
			if (tool.CanUseJSon())
			{
				if (!Core::JJSon::LoadData(*tool.json, eValue, key))
					return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
			}
			else
			{
				std::string guide;
				int value;
				(*tool.istream) >> guide >> value;
				eValue = (T)value;
			}
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		template<typename T>
		static Core::J_FILE_IO_RESULT StoreVector2(JFileIOTool& tool, const JVector2<T>& value, const std::string& guide)
		{
			if(!tool.CanStore())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			if constexpr (JVector2<T>::isValidValue)
			{
				if (tool.CanUseJSon())
				{
					std::stringstream stream;
					stream << value.x << " " << value.y;
					Core::JJSon::StoreData(*tool.json, stream.str(), guide);
				}
				else
					(*tool.ostream) << guide << " " << value.x << " " << value.y << '\n';
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT StoreVector3(JFileIOTool& tool, const JVector3<T>& value, const std::string& guide)
		{
			if constexpr (JVector3<T>::isValidValue)
			{
				if(!tool.CanStore())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				if (tool.CanUseJSon())
				{
					std::stringstream stream;
					stream << value.x << " " << value.y << " " << value.z;
					Core::JJSon::StoreData(*tool.json, stream.str(), guide);
				}
				else
					(*tool.ostream) << guide << " " << value.x << " " << value.y << " " << value.z << '\n';

				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT StoreVector4(JFileIOTool& tool, const JVector4<T>& value, const std::string& guide)
		{
			if constexpr (JVector4<T>::isValidValue)
			{
				if(!tool.CanStore())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				if (tool.CanUseJSon())
				{
					std::stringstream stream;
					stream << value.x << " " << value.y << " " << value.z << " " << value.w;
					Core::JJSon::StoreData(*tool.json, stream.str(), guide);
				}
				else
					(*tool.ostream) << guide << " " << value.x << " " << value.y << " " << value.z << " " << value.w << '\n';
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT LoadVector2(JFileIOTool& tool, JVector2<T>& v2, const std::string& key = "")
		{
			if constexpr (JVector2<T>::isValidValue)
			{
				if(!tool.CanLoad())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				if (tool.CanUseJSon())
				{
					std::string vecStr;
					if (!Core::JJSon::LoadData(*tool.json, vecStr, key))
						return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

					std::stringstream stream(vecStr);
					stream >> v2.x >> v2.y;
				}
				else
				{
					std::string guide;
					(*tool.istream) >> guide >> v2.x >> v2.y;
				}
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT LoadVector3(JFileIOTool& tool, JVector3<T>& v3, const std::string& key = "")
		{
			if constexpr (JVector3<T>::isValidValue)
			{
				if(!tool.CanLoad())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				if (tool.CanUseJSon())
				{
					std::string vecStr;
					if (!Core::JJSon::LoadData(*tool.json, vecStr, key))
						return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

					std::stringstream stream(vecStr);
					stream >> v3.x >> v3.y >> v3.z;
				}
				else
				{
					std::string guide;
					(*tool.istream) >> guide >> v3.x >> v3.y >> v3.z;
				}
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		template<typename T>
		static Core::J_FILE_IO_RESULT LoadVector4(JFileIOTool& tool, JVector4<T>& v4, const std::string& key = "")
		{
			if constexpr (JVector4<T>::isValidValue)
			{
				if(!tool.CanLoad())
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

				if (tool.CanUseJSon())
				{
					std::string vecStr;
					if (!Core::JJSon::LoadData(*tool.json, vecStr, key))
						return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

					std::stringstream stream(vecStr);
					stream >> v4.x >> v4.y >> v4.z >> v4.w;
				}
				else
				{
					std::string guide;
					(*tool.istream) >> guide >> v4.x >> v4.y >> v4.z >> v4.w;
				}
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
	public:
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT StoreAtomicDataVec(JFileIOTool& tool, const std::vector<T>& vec, int spaceoffset, const std::string& guide)
		{
			if(!tool.CanStore())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			
			tool.PushMember(guide);
			if (tool.CanUseJSon())
			{
				Core::JJSon::StoreData(*tool.json, spaceoffset, "SpaceOffset: ");
				Core::JJSon::StoreData(*tool.json, vec.size(), "VecCount: ");
				 
				int vecCount = (int)vec.size();
				int doneCount = 0;
				int groupCount = (vecCount / spaceoffset) + 1;

				tool.PushArrayOwner("Element");
				for (int i = 0; i < groupCount; ++i)
				{
					std::stringstream stream;
					for (int j = 0; j < spaceoffset && doneCount < vecCount; ++j, ++doneCount)
						stream << vec[i] << " ";

					tool.PushArrayMember();
					Core::JJSon::StoreData(*tool.json, stream.str(), std::to_string(i));
					tool.PopStack();
				}
				tool.PopStack(); 				 
			}
			else
			{ 
				(*tool.ostream) << "SpaceOffset: " << spaceoffset << '\n';
				(*tool.ostream) << "VecCount: " << vec.size() << '\n';
				(*tool.ostream) << "Element" << '\n';
				int vecCount = (int)vec.size();
				for (int i = 0; i < vecCount; ++i)
				{
					if (i % spaceoffset == 0 && i != 0)
						(*tool.ostream) << '\n';
					(*tool.ostream) << vec[i] << " ";
				}
				(*tool.ostream) << '\n';
			}
			tool.PopStack();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		static Core::J_FILE_IO_RESULT LoadAtomicDataVec(JFileIOTool& tool, std::vector<T>& vec, const std::string& key = "")
		{
			if(!tool.CanLoad())
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			int spaceOffset = 0;
			int vecCount = 0;
			bool isSuccess = true;
			tool.PushExistStack(key);
			if (tool.CanUseJSon())
			{ 
				isSuccess &= Core::JJSon::LoadData(*tool.json, spaceOffset, "SpaceOffset: ");
				isSuccess &= Core::JJSon::LoadData(*tool.json, vecCount, "VecCount: ");
				vec.resize(vecCount);

				int doneCount = 0;
				int groupCount = (vecCount / spaceOffset) + 1;
				tool.PushExistStack("Element");
				for (int i = 0; i < groupCount; ++i)
				{
					std::string indexStr;
					tool.PushExistStack();
					isSuccess &= Core::JJSon::LoadData(*tool.json, indexStr, std::to_string(i));
					tool.PopStack();

					std::stringstream stream(indexStr);
					while (stream >> vec[doneCount]) 
						++doneCount;
				}
				tool.PopStack();
			}
			else
			{
				std::string guide; 
				(*tool.istream) >> guide >> spaceOffset;
				(*tool.istream) >> guide >> vecCount;
				(*tool.istream) >> guide;

				vec.resize(vecCount);
				for (int i = 0; i < vecCount; ++i)
					(*tool.istream) >> vec[i];
			}
			tool.PopStack();
			return isSuccess ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
	public:
		static Core::J_FILE_IO_RESULT StoreMatrix4x4(JFileIOTool& tool, const JMatrix4x4& m, const std::string& guide);
		static Core::J_FILE_IO_RESULT LoadMatrix4x4(JFileIOTool& tool, JMatrix4x4& m, const std::string& key = "");
	public:
		static Core::J_FILE_IO_RESULT StoreJString(JFileIOTool& tool, const std::string& str, const std::string& guide);
		static Core::J_FILE_IO_RESULT StoreJString(JFileIOTool& tool, const std::wstring& wstr, const std::string& guide);
		static Core::J_FILE_IO_RESULT LoadJString(JFileIOTool& tool, std::string& str, const std::string& key = "");
		static Core::J_FILE_IO_RESULT LoadJString(JFileIOTool& tool, std::wstring& wstr, const std::string& key = "");
	public:
		static Core::J_FILE_IO_RESULT StoreXMFloat2(JFileIOTool& tool, const DirectX::XMFLOAT2& xm2, const std::string& guide);
		static Core::J_FILE_IO_RESULT StoreXMFloat3(JFileIOTool& tool, const DirectX::XMFLOAT3& xm3, const std::string& guide);
		static Core::J_FILE_IO_RESULT StoreXMFloat4(JFileIOTool& tool, const DirectX::XMFLOAT4& xm4, const std::string& guide);
		static Core::J_FILE_IO_RESULT StoreXMFloat4x4(JFileIOTool& tool, const DirectX::XMFLOAT4X4& xm4x4, const std::string& guide);
		static Core::J_FILE_IO_RESULT LoadXMFloat2(JFileIOTool& tool, DirectX::XMFLOAT2& xm2, const std::string& key = "");
		static Core::J_FILE_IO_RESULT LoadXMFloat3(JFileIOTool& tool, DirectX::XMFLOAT3& xm3, const std::string& key = "");
		static Core::J_FILE_IO_RESULT LoadXMFloat4(JFileIOTool& tool, DirectX::XMFLOAT4& xm4, const std::string& key = "");
		static Core::J_FILE_IO_RESULT LoadXMFloat4x4(JFileIOTool& tool, DirectX::XMFLOAT4X4& xm4x4, const std::string& key = "");
	public:
		static Core::J_FILE_IO_RESULT StoreIden(JFileIOTool& tool, Core::JIdentifier* obj);
		static Core::J_FILE_IO_RESULT LoadIden(JFileIOTool& tool, _Out_ size_t& oGuid);
		static Core::J_FILE_IO_RESULT LoadIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid);	public:
	public:
		static Core::J_FILE_IO_RESULT StoreFsmIden(JFileIOTool& tool, Core::JFSMinterface* obj);
		static Core::J_FILE_IO_RESULT LoadFsmIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ Core::J_FSM_OBJECT_TYPE& oType);
	public:
		static Core::J_FILE_IO_RESULT StoreHasIden(JFileIOTool& tool, Core::JIdentifier* iden, const std::string& guide, const bool applyUncopyableSymbol = false);
		static Core::J_FILE_IO_RESULT StoreHasInstanceHint(JFileIOTool& tool, const Core::JTypeInstanceSearchHint& hint, const std::string& guide, const bool applyUncopyableSymbol = false);
		//Warrning
		//Loading중 다른 오브젝트를 Load할 수 있으므로
		//Stream에러 발생 할 수있음
		//LoadHasObjectHint는 Hint만 전달하므로 Stream에러 발생에서 안전함
		static JUserPtr<Core::JIdentifier> LoadHasIden(JFileIOTool& tool, const std::string& key = "", TryAgainLoadObjIfFailPtr tryAgainPtr = nullptr, const bool applyUncopyableSymbol = false);
		template<typename T>
		static JUserPtr<T> LoadHasIden(JFileIOTool& tool, const std::string& key = "", TryAgainLoadObjIfFailPtr tryAgainPtr = nullptr)
		{
			return JUserPtr<T>::ConvertChild(LoadHasIden(tool, key, tryAgainPtr));
		}
		static Core::JTypeInstanceSearchHint LoadHasIdenHint(JFileIOTool& tool, const std::string& key = "", TryAgainLoadHintIfFailPtr tryAgainPtr = nullptr, const bool applyUncopyableSymbol = false);
	public:
		//skip file using getline until symbol
		//if fail stream is close
		static bool SkipLine(std::ifstream& stream, const std::string& symbol);
		//skip file using stream until symbol
		//if fail stream is close
		static bool SkipSentence(std::ifstream& stream, const std::string& symbol);
	public:
		static bool InputSpace(JFileIOTool& tool, int spaceCount);
	public:
		static Core::J_FILE_IO_RESULT CopyFile(const std::wstring& from, const std::wstring& to);
		//write to fixed symbol on toFile when end of  fromFile
		static Core::J_FILE_IO_RESULT CombineFile(const std::vector<std::wstring> from, const std::wstring& to);
		//read to fixed symbol on fromFile when end of toFile
		static Core::J_FILE_IO_RESULT DevideFile(const std::wstring& from, const std::vector<std::wstring> to);
		static Core::J_FILE_IO_RESULT DestroyFile(const std::wstring& path);
		static Core::J_FILE_IO_RESULT DestroyDirectory(const std::wstring& path);
		static bool HasFile(const std::wstring& path);
	public:
		/**
		* @return empty if invalid path
		*/
		static std::string FileToString(const std::string& path);
		static std::wstring FileToWString(const std::wstring& path);
	protected:
		static std::ifstream& GetInputStream(JFileIOTool& tool);
		static std::ofstream& GetOutputStream(JFileIOTool& tool);
		static Core::JJSon& GetJSonHandle(JFileIOTool& tool);
	};
}
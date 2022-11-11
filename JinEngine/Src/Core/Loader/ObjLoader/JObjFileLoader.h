#pragma once 
#include"JObjFileLoaderUtility.h"
#include"../../../Core/JDataType.h"
#include"../../../Core/Singleton/JSingletonHolder.h"  
#include<string>
#include<vector>

struct DirectX::BoundingBox;
struct DirectX::BoundingSphere;

namespace JinEngine
{   
	namespace Core
	{
		struct JFileImportPathData;
		class JObjFileLoaderImpl
		{
		private:
			struct FaceInfo
			{
			public:
				std::vector<DirectX::XMINT3> ptn;
				bool hasUV;
				bool hasNormal;
			public:
				FaceInfo(const std::vector<DirectX::XMINT3>& ptn);
			};
		public:
			bool LoadObjFile(const JFileImportPathData& pathData, JObjFileMeshData& objMeshData, std::vector<JObjFileMatData>& objMatData);
		private:
			bool LoadMatFile(const std::wstring& path, std::vector<JObjFileMatData>& objMatData);
			void GetVectorIndex(const std::wstring& wstr, int& posIndex, int& uvIndex, int& normalIndex)const noexcept;
			std::wstring GetMaterialPath(const std::wstring& folderPath, const std::wstring& name)const noexcept;
		};
	}
	using JObjFileLoader = Core::JSingletonHolder<Core::JObjFileLoaderImpl>;
}
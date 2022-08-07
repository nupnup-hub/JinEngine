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
	struct JModelAttribute;
	struct JResourcePathData;
	struct JStaticMeshData; 

	namespace Core
	{
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
			bool LoadObjFile(const JResourcePathData& pathData, std::vector<JObjMeshPartData>& objMeshData, std::vector<JObjMatData>& objMatData, JModelAttribute& modelAttribute);
		private:
			bool LoadMatFile(const std::wstring& path, std::vector<JObjMatData>& objMatData);
			void GetVectorIndex(const std::wstring& wstr, int& posIndex, int& uvIndex, int& normalIndex)const noexcept;
			std::string GetMaterialPath(const std::string& folderPath, const std::string& name)const noexcept;
		};
	}
	using JObjFileLoader = Core::JSingletonHolder<Core::JObjFileLoaderImpl>;
}
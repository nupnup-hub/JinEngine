#pragma once 
#include"JObjFileLoaderUtility.h" 
#include"../../../../Singleton/JSingletonHolder.h"  
#include<string>
#include<vector>

struct DirectX::BoundingBox;
struct DirectX::BoundingSphere;

namespace JinEngine
{ 
	namespace Core
	{
		struct JFileImportHelpData;
		using ObjMaterialMap = std::unordered_map<size_t, JObjFileMaterial>;
		using ObjMaterialRefMap = std::unordered_map<std::wstring, JObjFileMaterial&>;
		using ObjMaterialVec = std::vector<JObjFileMaterial>;

		class JObjFileLoader
		{
		private:
			struct FaceInfo
			{
			public:
				std::vector<JVector3<int>> ptn;
				bool hasUV;
				bool hasNormal;
			public:
				FaceInfo(const std::vector<JVector3<int>>& ptn);
			};
		public:
			bool LoadObjFile(const JFileImportHelpData& pathData, JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData);
		private:
			bool LoadMatFile(const std::wstring& path, ObjMaterialVec& materialVec, ObjMaterialRefMap& materialMap);
			void GetVectorIndex(const std::wstring& wstr, int& posIndex, int& uvIndex, int& normalIndex)const noexcept;
			std::wstring GetMaterialPath(const std::wstring& folderPath, const std::wstring& name)const noexcept;
		private:
			void DetectNan(float& x, float& y);
			void DetectNan(float& x, float& y, float& z);
		private:
			JVector3<float> SetLeftHand(const JVector3<float>& v)noexcept;
			size_t MakeVertexGuid(const uint x, const uint y, const uint z);
		};
	}
	using JObjFileLoader = Core::JSingletonHolder<Core::JObjFileLoader>;
}
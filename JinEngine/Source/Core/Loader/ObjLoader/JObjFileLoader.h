#pragma once 
#include"JObjFileLoaderUtility.h"
#include"../../../Core/JCoreEssential.h"
#include"../../../Core/Singleton/JSingletonHolder.h"  
#include<string>
#include<vector>

struct DirectX::BoundingBox;
struct DirectX::BoundingSphere;

namespace JinEngine
{   
	namespace Core
	{
		struct JFileImportHelpData;
		class JObjFileLoaderImpl
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
			bool LoadObjFile(const JFileImportHelpData& pathData, JObjFileMeshData& objMeshData, std::vector<JObjFileMatData>& objMatData);
		private:
			bool LoadMatFile(const std::wstring& path, std::vector<JObjFileMatData>& objMatData);
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
	using JObjFileLoader = Core::JSingletonHolder<Core::JObjFileLoaderImpl>;
}
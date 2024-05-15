#pragma once
#include"../../Core/JCoreEssential.h" 

namespace JinEngine
{
	namespace Graphic
	{
		namespace ShaderRelativePath
		{
			static std::wstring Image()
			{
				return L"Image";
			}
			static std::wstring Image(const std::wstring& name)
			{
				return Image() + L"\\" + name;
			}
			static std::wstring Ssao(const std::wstring& name)
			{
				return Image() + L"\\Ssao\\" + name;
			}
			static std::wstring Postprocessing(const std::wstring& name)
			{
				return Image() + L"\\Postprocessing\\" + name;
			}

			static std::wstring Culling()
			{
				return L"Culling";
			}
			static std::wstring Culling(const std::wstring& name)
			{
				return Culling() + L"\\" + name;
			}
			static std::wstring HZB(const std::wstring& name)
			{
				return Culling() + L"\\Occlusion\\" + name;
			}
			static std::wstring LightCulling(const std::wstring& name)
			{
				return Culling() + L"\\Light\\" + name;
			}

			static std::wstring Common()
			{
				return L"Common";
			}
			static std::wstring Common(const std::wstring& name)
			{
				return Common() + L"\\" + name;
			}
 
			static std::wstring ShadowMap()
			{
				return L"ShadowMap";
			}
			static std::wstring ShadowMap(const std::wstring& name)
			{
				return ShadowMap() + L"\\" + name;
			} 

			static std::wstring SceneRasterize()
			{
				return L"SceneRasterize";
			}
			static std::wstring SceneRasterize(const std::wstring& name)
			{
				return SceneRasterize() + L"\\" + name;
			}
			 
			static std::wstring Raytracing()
			{
				return L"Raytracing";
			}
			static std::wstring Raytracing(const std::wstring& name)
			{
				return Raytracing() + L"\\" + name;
			}
			 
			static std::wstring Restir()
			{
				return Raytracing() + L"\\Restir";
			}
			static std::wstring Restir(const std::wstring& name)
			{
				return Restir() + L"\\" + name;
			}		 
			static std::wstring RestirDenoise(const std::wstring& name)
			{
				return Restir() + L"\\Denoise\\" + name;
			}
			static std::wstring RestirGi(const std::wstring& name)
			{
				return Restir() + L"\\GI\\" + name;
			}
		}
	}
}
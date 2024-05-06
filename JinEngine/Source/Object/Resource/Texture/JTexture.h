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
#include"JTextureType.h"
#include"../JResourceObject.h"
#include"../JResourceObjectImporter.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceType.h" 
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../../Graphic/Image/JImageProcessingDesc.h"

namespace JinEngine
{

	class JTextureImportDesc : public JResourceObjectImportDesc
	{
		REGISTER_CLASS_ONLY_USE_TYPEINFO(JTextureImportDesc)
	public:
		bool useMipmap = false;		 
	public:
		JTextureImportDesc(const Core::JFileImportPathData& importPathData);
	};

	class JTexturePrivate;
	class JTexture final: public JResourceObject, public Graphic::JGraphicResourceUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JTexture) 
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			const std::wstring oridataPath;
		public:
			Graphic::J_GRAPHIC_RESOURCE_TYPE textureType;
			Graphic::JMipmapGenerationDesc mipMapDesc;
			Graphic::JConvertColorDesc convertDesc;
			J_TEXTURE_RESOLUTION resoultion;
		public:
			InitData(const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const std::wstring oridataPath,
				Graphic::J_GRAPHIC_RESOURCE_TYPE textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const std::wstring oridataPath,
				Graphic::J_GRAPHIC_RESOURCE_TYPE textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const std::wstring oridataPath,
				Graphic::J_GRAPHIC_RESOURCE_TYPE textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadMetaData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			Graphic::JMipmapGenerationDesc mipMapDesc;
			Graphic::JConvertColorDesc convertDesc;
			Graphic::J_GRAPHIC_RESOURCE_TYPE textureType;
			J_TEXTURE_RESOLUTION resoultion;
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JTexturePrivate;
		class JTextureImpl;
	private:
		std::unique_ptr<JTextureImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final; 
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::TEXTURE;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:	  
		uint GetTextureWidth()const noexcept;
		uint GetTextureHeight()const noexcept;
		Graphic::J_GRAPHIC_RESOURCE_TYPE GetTextureType()const noexcept;
		J_TEXTURE_RESOLUTION GetTextureResolution()const noexcept;
	public:
		void SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)noexcept;
		void SetTextureResolution(const J_TEXTURE_RESOLUTION resolutionType)noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;  
	private:
		JTexture(const InitData& initData);
		~JTexture();
	};
}
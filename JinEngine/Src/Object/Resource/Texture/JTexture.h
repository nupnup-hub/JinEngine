#pragma once
#include"../JResourceObject.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceType.h" 
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"

namespace JinEngine
{
	class JTexturePrivate;
	class JTexture final: public JResourceObject, public Graphic::JGraphicResourceUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JTexture)
		REGISTER_CLASS_IDENTIFIER_DEFAULT_LAZY_DESTRUCTION
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			const std::wstring oridataPath;
			Graphic::J_GRAPHIC_RESOURCE_TYPE textureType;
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
			Graphic::J_GRAPHIC_RESOURCE_TYPE textureType;
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JTexturePrivate;
		class JTextureImpl;
	private:
		std::unique_ptr<JTextureImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
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
		void SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;  
	private:
		JTexture(const InitData& initData);
		~JTexture();
	};
}
#pragma once
#include "../JFSMdiagramPrivate.h" 
#include"../../File/JFileIOResult.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	namespace Core
	{
		class JAnimationUpdateData;
		class JAnimationFSMdiagram;
		class JFSMdiagramOwnerInterface;
		class JAnimationFSMdiagramPrivate final: public JFSMdiagramPrivate
		{ 
		public:
			class AssetDataIOInterface
			{  
			private:
				friend class JAnimationController;
			private: 
				static JUserPtr<JAnimationFSMdiagram> LoadAssetData(std::wifstream& stream, JFSMdiagramOwnerInterface* fsmOwner);	//load diagram data and create state
				static J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMdiagram>& diagram);
			};
			class CreateInstanceInterface final : public JFSMdiagramPrivate::CreateInstanceInterface
			{
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)final;
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept final;
				bool CanCreateInstance(JDITypeDataBase* initData)const noexcept final;
			};
			class UpdateInterface final
			{
			private:
				friend class JAnimationController;
			private:
				static void Initialize(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber)noexcept;
				static void Enter(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber);
				static void Update(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		};
	}
}
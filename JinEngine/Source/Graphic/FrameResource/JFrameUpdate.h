#pragma once
#include"JFrameResourceEnum.h"
#include"../JGraphicConstants.h"  
#include"../../Core/Empty/JEmptyType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{ 
#pragma region IFramaeUpdate

		class JFrameUpdateData
		{ 
		private:
			//same as holdervec index
			int number = 0;
		private:
			//frame index	calculated) index = pre data index + pre data indexSize
			int index = -1;
			//frame index size
			short indexSize = -1;
		private:
			int8 movedDataDirty = 0;
			uint8 sortOrder = UCHAR_MAX;		//ascending order number
		protected:
			JFrameUpdateData() = default;
			virtual ~JFrameUpdateData() = default;
		public:
			int GetFrameIndex()const noexcept; 
			int GetFrameIndexSize()const noexcept;
			static uint GetTotalRegistedCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type);
			static uint GetTotalFrameCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type);
			static uint GetAreaRegistedCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid);
			//count 0 ~ areaStart
			static uint GetAreaRegistedOffset(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid);
		protected:
			void SetUploadIndex(const int value) noexcept; 
			void SetMovedDirty()noexcept;
			void MinusMovedDirty()noexcept;
		public:
			bool HasValidFrameIndex()const noexcept;
			bool HasMovedDirty()const noexcept;
		protected:
			//areaGuid는 scene별 혹은 다른 object별 frameData를 구분해 정렬시키기 위해 사용된다
			static void RegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, 
				JFrameUpdateData* holder,
				const size_t areaGuid,
				const ushort indexSize = 1,
				const uint8 sortOrder = 255);
			static void DeRegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder);
			static void ReRegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder);
		private:
			//area count == 0 일시
			static bool PushBack(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder, const size_t areaGuid);
			static bool Insert(const J_UPLOAD_FRAME_RESOURCE_TYPE type, 
				JFrameUpdateData* holder, 
				const size_t areaGuid,
				const int areaStIndex,
				const int areaEdIndex);
			static bool Pop(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder); 
		};

		template<J_UPLOAD_FRAME_RESOURCE_TYPE uploadType, typename ...Param>
		class JFrameUpdateInterface : public JFrameUpdateData
		{  
		protected:
			virtual ~JFrameUpdateInterface() = default;
		protected:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetUploadType()const noexcept
			{
				return uploadType;
			}
		private:
			virtual void UpdateFrame(Param... var)noexcept = 0;
		};

		template<typename JFrameUpdateInterface1>
		class JFrameUpdateInterfaceHolder1 : public JFrameUpdateInterface1
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2>
		class JFrameUpdateInterfaceHolder2 : public JFrameUpdateInterface1,
			public JFrameUpdateInterface2
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
			using JFrameInterface2 = JFrameUpdateInterface2;
		public:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
			{
				return JFrameInterface1::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
			{
				return JFrameInterface2::GetUploadType();
			}
			uint GetFirstFrameIndex()const noexcept
			{
				return JFrameInterface1::GetFrameIndex();
			}
			uint GetSecondFrameIndex()const noexcept
			{
				return JFrameInterface2::GetFrameIndex();
			}
		protected:
			void SetFirstFrameIndex(const uint value)noexcept
			{
				JFrameInterface1::SetUploadIndex(value);
			}
			void SetSecondFrameIndex(const uint value)noexcept
			{
				JFrameInterface2::SetUploadIndex(value);
			}
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3>
		class JFrameUpdateInterfaceHolder3 : public JFrameUpdateInterface1,
			public JFrameUpdateInterface2,
			public JFrameUpdateInterface3
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
			using JFrameInterface2 = JFrameUpdateInterface2;
			using JFrameInterface3 = JFrameUpdateInterface3;
		public:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
			{
				return JFrameInterface1::GetUploadType();
			}
			uint GetFirstFrameIndex()const noexcept
			{
				return JFrameInterface1::GetFrameIndex();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
			{
				return JFrameInterface2::GetUploadType();
			}
			uint GetSecondFrameIndex()const noexcept
			{
				return JFrameInterface2::GetFrameIndex();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
			{
				return JFrameInterface3::GetUploadType();
			}
			uint GetThirdFrameIndex()const noexcept
			{
				return JFrameInterface3::GetFrameIndex();
			}
		protected:
			void SetFirstFrameIndex(const uint value)noexcept
			{
				JFrameInterface1::SetUploadIndex(value);
			}
			void SetSecondFrameIndex(const uint value)noexcept
			{
				JFrameInterface2::SetUploadIndex(value);
			}
			void SetThirdFrameIndex(const uint value)noexcept
			{
				JFrameInterface3::SetUploadIndex(value);
			}
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3, typename JFrameUpdateInterface4>
		class JFrameUpdateInterfaceHolder4 : public JFrameUpdateInterface1,
			public JFrameUpdateInterface2,
			public JFrameUpdateInterface3,
			public JFrameUpdateInterface4
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
			using JFrameInterface2 = JFrameUpdateInterface2;
			using JFrameInterface3 = JFrameUpdateInterface3;
			using JFrameInterface4 = JFrameUpdateInterface4;
		protected:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
			{
				return JFrameInterface1::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
			{
				return JFrameInterface2::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
			{
				return JFrameInterface3::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetForthUploadType()const noexcept
			{
				return JFrameInterface4::GetUploadType();
			}
			uint GetFirstFrameIndex()const noexcept
			{
				return JFrameInterface1::GetFrameIndex();
			}
			uint GetSecondFrameIndex()const noexcept
			{
				return JFrameInterface2::GetFrameIndex();
			}
			uint GetThirdFrameIndex()const noexcept
			{
				return JFrameInterface3::GetFrameIndex();
			}
			uint GetForthFrameIndex()const noexcept
			{
				return JFrameInterface4::GetFrameIndex();
			}
		protected:
			void SetFirstFrameIndex(const uint value)noexcept
			{
				JFrameInterface1::SetUploadIndex(value);
			}
			void SetSecondFrameIndex(const uint value)noexcept
			{
				JFrameInterface2::SetUploadIndex(value);
			}
			void SetThirdFrameIndex(const uint value)noexcept
			{
				JFrameInterface3::SetUploadIndex(value);
			}
			void SetForthFrameIndex(const uint value)noexcept
			{
				JFrameInterface4::SetUploadIndex(value);
			}
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3, typename JFrameUpdateInterface4, typename JFrameUpdateInterface5>
		class JFrameUpdateInterfaceHolder5 : public JFrameUpdateInterface1,
			public JFrameUpdateInterface2,
			public JFrameUpdateInterface3,
			public JFrameUpdateInterface4,
			public JFrameUpdateInterface5
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
			using JFrameInterface2 = JFrameUpdateInterface2;
			using JFrameInterface3 = JFrameUpdateInterface3;
			using JFrameInterface4 = JFrameUpdateInterface4;
			using JFrameInterface5 = JFrameUpdateInterface5;
		 protected:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
			{
				return JFrameInterface1::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
			{
				return JFrameInterface2::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
			{
				return JFrameInterface3::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetForthUploadType()const noexcept
			{
				return JFrameInterface4::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFifthUploadType()const noexcept
			{
				return JFrameInterface5::GetUploadType();
			}
			uint GetFirstFrameIndex()const noexcept
			{
				return JFrameInterface1::GetFrameIndex();
			}
			uint GetSecondFrameIndex()const noexcept
			{
				return JFrameInterface2::GetFrameIndex();
			}
			uint GetThirdFrameIndex()const noexcept
			{
				return JFrameInterface3::GetFrameIndex();
			}
			uint GetForthFrameIndex()const noexcept
			{
				return JFrameInterface4::GetFrameIndex();
			}
			uint GetFifthFrameIndex()const noexcept
			{
				return JFrameInterface5::GetFrameIndex();
			}
		protected:
			void SetFirstFrameIndex(const uint value)noexcept
			{
				JFrameInterface1::SetUploadIndex(value);
			}
			void SetSecondFrameIndex(const uint value)noexcept
			{
				JFrameInterface2::SetUploadIndex(value);
			}
			void SetThirdFrameIndex(const uint value)noexcept
			{
				JFrameInterface3::SetUploadIndex(value);
			}
			void SetForthFrameIndex(const uint value)noexcept
			{
				JFrameInterface4::SetUploadIndex(value);
			}
			void SetFifthFrameIndex(const uint value)noexcept
			{
				JFrameInterface5::SetUploadIndex(value);
			}
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3, typename JFrameUpdateInterface4, typename JFrameUpdateInterface5,typename JFrameUpdateInterface6>
		class JFrameUpdateInterfaceHolder6 : public JFrameUpdateInterface1,
			public JFrameUpdateInterface2,
			public JFrameUpdateInterface3,
			public JFrameUpdateInterface4,
			public JFrameUpdateInterface5,
			public JFrameUpdateInterface6
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
			using JFrameInterface2 = JFrameUpdateInterface2;
			using JFrameInterface3 = JFrameUpdateInterface3;
			using JFrameInterface4 = JFrameUpdateInterface4;
			using JFrameInterface5 = JFrameUpdateInterface5;
			using JFrameInterface6 = JFrameUpdateInterface6;
		protected:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
			{
				return JFrameInterface1::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
			{
				return JFrameInterface2::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
			{
				return JFrameInterface3::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetForthUploadType()const noexcept
			{
				return JFrameInterface4::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFifthUploadType()const noexcept
			{
				return JFrameInterface5::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSixthUploadType()const noexcept
			{
				return JFrameInterface6::GetUploadType();
			}
			uint GetFirstFrameIndex()const noexcept
			{
				return JFrameInterface1::GetFrameIndex();
			}
			uint GetSecondFrameIndex()const noexcept
			{
				return JFrameInterface2::GetFrameIndex();
			}
			uint GetThirdFrameIndex()const noexcept
			{
				return JFrameInterface3::GetFrameIndex();
			}
			uint GetForthFrameIndex()const noexcept
			{
				return JFrameInterface4::GetFrameIndex();
			}
			uint GetFifthFrameIndex()const noexcept
			{
				return JFrameInterface5::GetFrameIndex();
			}
			uint GetSixthFrameIndex()const noexcept
			{
				return JFrameInterface6::GetFrameIndex();
			}
		protected:
			void SetFirstFrameIndex(const uint value)noexcept
			{
				JFrameInterface1::SetUploadIndex(value);
			}
			void SetSecondFrameIndex(const uint value)noexcept
			{
				JFrameInterface2::SetUploadIndex(value);
			}
			void SetThirdFrameIndex(const uint value)noexcept
			{
				JFrameInterface3::SetUploadIndex(value);
			}
			void SetForthFrameIndex(const uint value)noexcept
			{
				JFrameInterface4::SetUploadIndex(value);
			}
			void SetFifthFrameIndex(const uint value)noexcept
			{
				JFrameInterface5::SetUploadIndex(value);
			}
			void SetSixthFrameIndex(const uint value)noexcept
			{
				JFrameInterface6::SetUploadIndex(value);
			}
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3, typename JFrameUpdateInterface4, typename JFrameUpdateInterface5, typename JFrameUpdateInterface6, typename JFrameUpdateInterface7>
		class JFrameUpdateInterfaceHolder7 : public JFrameUpdateInterface1,
			public JFrameUpdateInterface2,
			public JFrameUpdateInterface3,
			public JFrameUpdateInterface4,
			public JFrameUpdateInterface5,
			public JFrameUpdateInterface6,
			public JFrameUpdateInterface7
		{
		protected:
			using JFrameInterface1 = JFrameUpdateInterface1;
			using JFrameInterface2 = JFrameUpdateInterface2;
			using JFrameInterface3 = JFrameUpdateInterface3;
			using JFrameInterface4 = JFrameUpdateInterface4;
			using JFrameInterface5 = JFrameUpdateInterface5;
			using JFrameInterface6 = JFrameUpdateInterface6;
			using JFrameInterface7 = JFrameUpdateInterface7;
		protected:
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
			{
				return JFrameInterface1::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
			{
				return JFrameInterface2::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
			{
				return JFrameInterface3::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetForthUploadType()const noexcept
			{
				return JFrameInterface4::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetFifthUploadType()const noexcept
			{
				return JFrameInterface5::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSixthUploadType()const noexcept
			{
				return JFrameInterface6::GetUploadType();
			}
			J_UPLOAD_FRAME_RESOURCE_TYPE GetSeventhUploadType()const noexcept
			{
				return JFrameInterface7::GetUploadType();
			}
			uint GetFirstFrameIndex()const noexcept
			{
				return JFrameInterface1::GetFrameIndex();
			}
			uint GetSecondFrameIndex()const noexcept
			{
				return JFrameInterface2::GetFrameIndex();
			}
			uint GetThirdFrameIndex()const noexcept
			{
				return JFrameInterface3::GetFrameIndex();
			}
			uint GetForthFrameIndex()const noexcept
			{
				return JFrameInterface4::GetFrameIndex();
			}
			uint GetFifthFrameIndex()const noexcept
			{
				return JFrameInterface5::GetFrameIndex();
			}
			uint GetSixthFrameIndex()const noexcept
			{
				return JFrameInterface6::GetFrameIndex();
			}
			uint GetSeventhFrameIndex()const noexcept
			{
				return JFrameInterface7::GetFrameIndex();
			}
		protected:
			void SetFirstFrameIndex(const uint value)noexcept
			{
				JFrameInterface1::SetUploadIndex(value);
			}
			void SetSecondFrameIndex(const uint value)noexcept
			{
				JFrameInterface2::SetUploadIndex(value);
			}
			void SetThirdFrameIndex(const uint value)noexcept
			{
				JFrameInterface3::SetUploadIndex(value);
			}
			void SetForthFrameIndex(const uint value)noexcept
			{
				JFrameInterface4::SetUploadIndex(value);
			}
			void SetFifthFrameIndex(const uint value)noexcept
			{
				JFrameInterface5::SetUploadIndex(value);
			}
			void SetSixthFrameIndex(const uint value)noexcept
			{
				JFrameInterface6::SetUploadIndex(value);
			}
			void SetSeventhFrameIndex(const uint value)noexcept
			{
				JFrameInterface7::SetUploadIndex(value);
			}
		private:
			virtual void UpdateFrameEnd() = 0;
		};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3, typename JFrameUpdateInterface4, typename JFrameUpdateInterface5, typename JFrameUpdateInterface6, typename JFrameUpdateInterface7, typename JFrameUpdateInterface8>
		class JFrameUpdateInterfaceHolder8 : public JFrameUpdateInterface1,
				public JFrameUpdateInterface2,
				public JFrameUpdateInterface3,
				public JFrameUpdateInterface4,
				public JFrameUpdateInterface5,
				public JFrameUpdateInterface6,
				public JFrameUpdateInterface7,
				public JFrameUpdateInterface8
			{
			protected:
				using JFrameInterface1 = JFrameUpdateInterface1;
				using JFrameInterface2 = JFrameUpdateInterface2;
				using JFrameInterface3 = JFrameUpdateInterface3;
				using JFrameInterface4 = JFrameUpdateInterface4;
				using JFrameInterface5 = JFrameUpdateInterface5;
				using JFrameInterface6 = JFrameUpdateInterface6;
				using JFrameInterface7 = JFrameUpdateInterface7;
				using JFrameInterface8 = JFrameUpdateInterface8;
			protected:
				J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
				{
					return JFrameInterface1::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
				{
					return JFrameInterface2::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
				{
					return JFrameInterface3::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetForthUploadType()const noexcept
				{
					return JFrameInterface4::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetFifthUploadType()const noexcept
				{
					return JFrameInterface5::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetSixthUploadType()const noexcept
				{
					return JFrameInterface6::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetSeventhUploadType()const noexcept
				{
					return JFrameInterface7::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetEighthUploadType()const noexcept
				{
					return JFrameInterface8::GetUploadType();
				}
				uint GetFirstFrameIndex()const noexcept
				{
					return JFrameInterface1::GetFrameIndex();
				}
				uint GetSecondFrameIndex()const noexcept
				{
					return JFrameInterface2::GetFrameIndex();
				}
				uint GetThirdFrameIndex()const noexcept
				{
					return JFrameInterface3::GetFrameIndex();
				}
				uint GetForthFrameIndex()const noexcept
				{
					return JFrameInterface4::GetFrameIndex();
				}
				uint GetFifthFrameIndex()const noexcept
				{
					return JFrameInterface5::GetFrameIndex();
				}
				uint GetSixthFrameIndex()const noexcept
				{
					return JFrameInterface6::GetFrameIndex();
				}
				uint GetSeventhFrameIndex()const noexcept
				{
					return JFrameInterface7::GetFrameIndex();
				}
				uint GetEighthFrameIndex()const noexcept
				{
					return JFrameInterface8::GetFrameIndex();
				}
			protected:
				void SetFirstFrameIndex(const uint value)noexcept
				{
					JFrameInterface1::SetUploadIndex(value);
				}
				void SetSecondFrameIndex(const uint value)noexcept
				{
					JFrameInterface2::SetUploadIndex(value);
				}
				void SetThirdFrameIndex(const uint value)noexcept
				{
					JFrameInterface3::SetUploadIndex(value);
				}
				void SetForthFrameIndex(const uint value)noexcept
				{
					JFrameInterface4::SetUploadIndex(value);
				}
				void SetFifthFrameIndex(const uint value)noexcept
				{
					JFrameInterface5::SetUploadIndex(value);
				}
				void SetSixthFrameIndex(const uint value)noexcept
				{
					JFrameInterface6::SetUploadIndex(value);
				}
				void SetSeventhFrameIndex(const uint value)noexcept
				{
					JFrameInterface7::SetUploadIndex(value);
				}
				void SetEighthFrameIndex(const uint value)noexcept
				{
					JFrameInterface8::SetUploadIndex(value);
				}
			private:
				virtual void UpdateFrameEnd() = 0;
			};
		template<typename JFrameUpdateInterface1, typename JFrameUpdateInterface2, typename JFrameUpdateInterface3, typename JFrameUpdateInterface4, typename JFrameUpdateInterface5, typename JFrameUpdateInterface6, typename JFrameUpdateInterface7, typename JFrameUpdateInterface8, typename JFrameUpdateInterface9>
		class JFrameUpdateInterfaceHolder9 : public JFrameUpdateInterface1,
				public JFrameUpdateInterface2,
				public JFrameUpdateInterface3,
				public JFrameUpdateInterface4,
				public JFrameUpdateInterface5,
				public JFrameUpdateInterface6,
				public JFrameUpdateInterface7,
				public JFrameUpdateInterface8,
				public JFrameUpdateInterface9
			{
			protected:
				using JFrameInterface1 = JFrameUpdateInterface1;
				using JFrameInterface2 = JFrameUpdateInterface2;
				using JFrameInterface3 = JFrameUpdateInterface3;
				using JFrameInterface4 = JFrameUpdateInterface4;
				using JFrameInterface5 = JFrameUpdateInterface5;
				using JFrameInterface6 = JFrameUpdateInterface6;
				using JFrameInterface7 = JFrameUpdateInterface7;
				using JFrameInterface8 = JFrameUpdateInterface8;
				using JFrameInterface9 = JFrameUpdateInterface9;
			protected:
				J_UPLOAD_FRAME_RESOURCE_TYPE GetFirstUploadType()const noexcept
				{
					return JFrameInterface1::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetSecondUploadType()const noexcept
				{
					return JFrameInterface2::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetThirdUploadType()const noexcept
				{
					return JFrameInterface3::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetForthUploadType()const noexcept
				{
					return JFrameInterface4::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetFifthUploadType()const noexcept
				{
					return JFrameInterface5::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetSixthUploadType()const noexcept
				{
					return JFrameInterface6::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetSeventhUploadType()const noexcept
				{
					return JFrameInterface7::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetEighthUploadType()const noexcept
				{
					return JFrameInterface8::GetUploadType();
				}
				J_UPLOAD_FRAME_RESOURCE_TYPE GetNinethUploadType()const noexcept
				{
					return JFrameInterface9::GetUploadType();
				}
				uint GetFirstFrameIndex()const noexcept
				{
					return JFrameInterface1::GetFrameIndex();
				}
				uint GetSecondFrameIndex()const noexcept
				{
					return JFrameInterface2::GetFrameIndex();
				}
				uint GetThirdFrameIndex()const noexcept
				{
					return JFrameInterface3::GetFrameIndex();
				}
				uint GetForthFrameIndex()const noexcept
				{
					return JFrameInterface4::GetFrameIndex();
				}
				uint GetFifthFrameIndex()const noexcept
				{
					return JFrameInterface5::GetFrameIndex();
				}
				uint GetSixthFrameIndex()const noexcept
				{
					return JFrameInterface6::GetFrameIndex();
				}
				uint GetSeventhFrameIndex()const noexcept
				{
					return JFrameInterface7::GetFrameIndex();
				}
				uint GetEighthFrameIndex()const noexcept
				{
					return JFrameInterface8::GetFrameIndex();
				}
				uint GetNinethFrameIndex()const noexcept
				{
					return JFrameInterface9::GetFrameIndex();
				}
			protected:
				void SetFirstFrameIndex(const uint value)noexcept
				{
					JFrameInterface1::SetUploadIndex(value);
				}
				void SetSecondFrameIndex(const uint value)noexcept
				{
					JFrameInterface2::SetUploadIndex(value);
				}
				void SetThirdFrameIndex(const uint value)noexcept
				{
					JFrameInterface3::SetUploadIndex(value);
				}
				void SetForthFrameIndex(const uint value)noexcept
				{
					JFrameInterface4::SetUploadIndex(value);
				}
				void SetFifthFrameIndex(const uint value)noexcept
				{
					JFrameInterface5::SetUploadIndex(value);
				}
				void SetSixthFrameIndex(const uint value)noexcept
				{
					JFrameInterface6::SetUploadIndex(value);
				}
				void SetSeventhFrameIndex(const uint value)noexcept
				{
					JFrameInterface7::SetUploadIndex(value);
				}
				void SetEighthFrameIndex(const uint value)noexcept
				{
					JFrameInterface8::SetUploadIndex(value);
				}
				void SetNinethFrameIndex(const uint value)noexcept
				{
					JFrameInterface9::SetUploadIndex(value);
				}
			private:
				virtual void UpdateFrameEnd() = 0;
			};
#pragma endregion

#pragma region IFrameDirty

		class JFrameDirtyTriggerBase
		{
		protected:
			virtual ~JFrameDirtyTriggerBase() = default;
		public:
			virtual void SetFrameDirty()noexcept = 0;
		};

		//just call empty func
		class JFrameDirtyTrigger : public JFrameDirtyTriggerBase
		{
		public:
			void SetFrameDirty()noexcept override;
		};

		//주로 FrameeResource가아닌 자원이 사용되는경우 할당된다
		//ex) shadowMap, occlusion
		//이들은 프레임개수만큼이 아닌 한번만 업데이트가 필요하고 업데이트 주기는 FrameResource와 마찬가지이나
		//추가로 이들만 업데이트가 필요한 경우가있기에 클래스를 확장하여 해결한다.

		class JFrameDirtyBase
		{
		protected:
			virtual ~JFrameDirtyBase() = default;
		public:
			virtual bool IsFrameDirted()const noexcept = 0;
		};
		 
		class JFrameDirty: public JFrameDirtyBase, public JFrameDirtyTriggerBase
		{
		private:
			int frameDirty = 0;
			bool isLastHotUpdated = false;
			bool isLastUpdated = false;
		public:
			int GetFrameDirty()noexcept;
			void SetFrameDirty()noexcept override;
		public:
			void SetLastFrameHotUpdatedTrigger(const bool value)noexcept;
			void SetLastFrameUpdatedTrigger(const bool value)noexcept;
		public:
			bool IsFrameDirted()const noexcept final;
			bool IsLastFrameHotUpdated()const noexcept;
			bool IsLastFrameUpdated()const noexcept;
		public:
			void MinusFrameDirty()noexcept;
			void OffFrameDirty()noexcept;
		};

		template<typename FrameDirty>
		class JFrameDirtyChain : public FrameDirty
		{
		private:
			struct ListenerInfo
			{
			public:
				JFrameDirtyTriggerBase* dirty;
				size_t guid;
			public:
				ListenerInfo(JFrameDirtyTriggerBase* dirty, const size_t guid)
					:dirty(dirty), guid(guid)
				{} 
			};
		private:
			std::vector<ListenerInfo> listener;
		protected:
			void SetFrameDirty()noexcept final
			{
				FrameDirty::SetFrameDirty();
				const uint listenerCount = (uint)listener.size();
				for (uint i = 0; i < listenerCount; ++i)
					listener[i].dirty->SetFrameDirty();
			}
		protected:
			void AddFrameDirtyListener(JFrameDirtyTriggerBase* newListener, const size_t guid)noexcept
			{
				listener.push_back(ListenerInfo(newListener, guid));
			}
			void RemoveFrameDirtyListener(const size_t guid)noexcept
			{
				uint listenerCount = (uint)listener.size();
				for (uint i = 0; i < listenerCount; ++i)
				{
					if (listener[i].guid == guid)
					{
						listener.erase(listener.begin() + i);
						break;
					}
				}
			}
		};

#pragma endregion
 
		template<typename IFrameUpdate, typename JFrameDirtyBase>
		class JFrameUpdate;
		 
		//use dirty
		template<typename FrameUpdate, typename FrameDirty>
		class JFrameUpdate : public FrameUpdate, public FrameDirty
		{
		public:
			void UpdateFrameEnd()override
			{
				FrameDirty::MinusFrameDirty();
			}
		};
  
		//unuse dirty
		template<typename FrameUpdate>
		class JFrameUpdate<FrameUpdate, Core::JEmptyType> : public FrameUpdate
		{
		public:
			void UpdateFrameEnd()override
			{}
		};
	}
}
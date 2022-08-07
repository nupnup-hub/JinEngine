#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicBufManagerInterface;
		class JGraphicBufElementInterface
		{
		private:
			friend class JGraphicBufUserInterface;
			friend class JGraphicBufManagerInterface;
		private:
			int buffIndex;
		public:
			virtual ~JGraphicBufElementInterface() = default;
		protected:
			int GetBuffIndex()const noexcept;
			void SetBuffIndex(const int value)noexcept;
		};

		class JGraphicBufUserInterface
		{
		public:
			virtual ~JGraphicBufUserInterface() = default;
		protected:
			int GetBuffIndex(JGraphicBufElementInterface& bufEleInterface)const noexcept;
		};
		class JGraphicBufManagerInterface : public JGraphicBufUserInterface
		{
		protected:
			void SetBuffIndex(JGraphicBufElementInterface& bufEleInterface, int index)noexcept;
		};
	}
}
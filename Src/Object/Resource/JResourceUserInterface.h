#pragma once
namespace JinEngine
{
	class JReferenceInterface;
	class JValidInterface;

	class JResourceUserInterface
	{
	protected:
		void OnResourceReference(JReferenceInterface& jRobj);
		void OffResourceReference(JReferenceInterface& jRobj);
		int GetResourceReferenceCount(JReferenceInterface& jRobj);
		bool IsVaildResource(JValidInterface& jRobj);
	};
}
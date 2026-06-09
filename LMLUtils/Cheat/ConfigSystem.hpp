#pragma once

#include <string>
#include <vector>
#include <set>

#include <FrameWork/FrameWork.hpp>
#include <FrameWork/Utilities/Base64.hpp>

#include "Options.hpp"

namespace Cheat
{
	class ConfigManager
	{
	public:
		class CConfigItem
		{
		public:
			std::string Name;
			void* Pointer;
			std::string Type;

			CConfigItem(std::string Name, void* Pointer, std::string Type)
			{
				this->Name = Name;
				this->Pointer = Pointer;
				this->Type = Type;
			}
		};

		std::vector<CConfigItem*> Items;

		void AddItem(void* Pointer, const char* Name, const std::string& Type);

		void SetupItem(int* Pointer, float Value, const std::string& Name);

		void SetupItem(float* Pointer, float Value, const std::string& Name);

		void SetupItem(bool* Pointer, float Value, const std::string& Name);

		void SetupItem(float(*Pointer)[4], float v0, float v1, float v2, float v3, const std::string& Name);

		void SetupItem(float(*Pointer)[2], float v0, float v1, const std::string& Name);

		void SetupItem(char(*Pointer)[64], const char* Value, const std::string& Name);

		void SetupItem(uint64_t* Pointer, uint64_t Value, const std::string& Name);

		void SetupItem(std::string* Pointer, const std::string& Value, const std::string& Name);

		void Setup();

		ConfigManager()
		{
			Setup();
		};

		void ExportToClipboard();
		void ImportFromClipboard();
		void SaveToFile();
		void LoadFromFile();
		std::vector<std::string> ListSlots();
		void SaveSlot(const std::string& name);
		void LoadSlot(const std::string& name);
		void DeleteSlot(const std::string& name);
	};
}
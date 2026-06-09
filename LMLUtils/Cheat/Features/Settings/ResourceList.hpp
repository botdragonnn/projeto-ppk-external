#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <atomic>
#include "../../FivemSDK/Fivem.hpp"

namespace Cheat
{
	namespace ResourceManager
	{
		enum eResourceState : uint32_t
		{
			Uninitialized,
			Stopped,
			Starting,
			Started,
			Stopping
		};

		struct Resources_t
		{
			uintptr_t Pointer = 0;
			std::string Name;
			std::string Path;
			eResourceState State = eResourceState::Uninitialized;
		};

		inline std::vector<Resources_t> vResources;
		inline std::mutex g_ResourceMutex;

		class cResourceList
		{
		public:
			void Refresh();
			bool Stop(uintptr_t ResourcePtr);
			void RunThread();
			uint64_t GetManagerAddr() { return m_ManagerAddr; }

		private:
			uint64_t m_ManagerAddr = 0;
			uint64_t m_LastRefresh = 0;
			std::atomic<bool> m_Initialized{ false };

			bool FindManager();
			std::string ReadResourceName(uintptr_t resourcePtr);
		};

		inline cResourceList g_ResourceList;
	}
}

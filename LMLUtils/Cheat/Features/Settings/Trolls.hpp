#pragma once

#include <Windows.h>
#include "../../FivemSDK/Fivem.hpp"

namespace Cheat
{
	namespace Trolls
	{
		void GrabVehicle();
		void ReleaseVehicle(bool throwVehicle);
		bool IsHoldingVehicle();
		void RunThread();
	}
}

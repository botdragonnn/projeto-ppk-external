#pragma once
#include <cstdint>
#include <mutex>

#include "Classes.hpp"

#include <FrameWork/FrameWork.hpp>

namespace Cheat
{
	enum GAME_VERSION
	{
		GAME_VERSION_GAME_b2372,
		GAME_VERSION_GTA_b2372,
		GAME_VERSION_GAME_b2612,
		GAME_VERSION_GTA_b2612,
		GAME_VERSION_GAME_b2699,
		GAME_VERSION_GTA_b2699,
		GAME_VERSION_GAME_b2189,
		GAME_VERSION_GTA_b2189,
		GAME_VERSION_GAME_b2802,
		GAME_VERSION_GTA_b2802,
		GAME_VERSION_GAME_b2060,
		GAME_VERSION_GTA_b2060,
		GAME_VERSION_GAME_b2545,
		GAME_VERSION_GAME_b3407,
		GAME_VERSION_GTA_b3407,
		GAME_VERSION_GTA_b2545,
		GAME_VERSION_GAME_b2944,
		GAME_VERSION_GAME_b3258,
		GAME_VERSION_GTA_b2944,
		GAME_VERSION_GTA_b3258,
		GAME_VERSION_GAME_b3095,
		GAME_VERSION_GTA_b3095,
		GAME_VERSION_GAME_b3570,
		GAME_VERSION_GTA_b3570,
	};

	struct PedStaticInfo
	{
		CPed* Ped = nullptr;
		int iIndex = -1;
		int NetId = -1;
		bool bIsLocalPlayer = false;
		bool bIsNPC = false;
		std::string Name = "";
		uint64_t crSkeletonData = 0;
		bool bIsFriend = false;
		uint64_t PlayerInfoAddr = 0;
		uint32_t IpAddress = 0;
		std::string DiscordId = "";
		std::string SteamId = "";

		std::unordered_map<unsigned int, unsigned> MaskToBoneId;
	};

	struct Entity
	{
		PedStaticInfo StaticInfo;
		Vector3D Cordinates = Vector3D(0, 0, 0);
		bool Visible = false;
		ImVec2 HeadPos = ImVec2(0, 0);
	};

	struct LocalPEDInfo
	{
		CPed* Ped = nullptr;
		int iIndex = -1;
		ImVec2 ScreenPos = ImVec2(0, 0);
		Vector3D WorldPos = Vector3D(0, 0, 0);
	};

	struct VehicleInfo
	{
		CVehicle* Vehicle = nullptr;
		std::string Name = "";
		uint64_t ModelInfo = 0;
		int iIndex = -1;
		uintptr_t ptr = 0;
	};

	float GetPlayerHeading();

	class FivemSDK
	{
	public:
		// Construtor - inicializa TODAS as variáveis
		FivemSDK()
			: bIsIntialized(false)
			, World(0)
			, ReplayInterface(0)
			, ViewPort(0)
			, Camera(0)
			, bIsPlayerAiming(0)
			, PlayerAimingAt(0)
			, HandleBullet(0)
			, GameplayCamHolder(0)
			, GameplayCamTarget(0)
			, CanCombatRoll(0)
			, UpdateCamBasePosition(0)
			, BlipList(0)
			, NetIdToNamesPtr(0)
			, CitizemPlayerNamesModule(0)
			, RequestRagdoll(0)
			, Pid(0)
			, ModuleBase(0)
			, ModuleBaseSize(0)
			, LanGame(false)
			, GameVersion(0)
			, RealGameVersion(0)
			, pWorld(nullptr)
			, pLocalPlayer(nullptr)
			, pReplayInterface(nullptr)
			, pPedInterface(nullptr)
			, pVehicleInterface(nullptr)
			, pCamGameplayDirector(nullptr)
			, pViewPort(0)
			, ProcHandle(nullptr)
			, WorldProbeAddr(0)
			, GetShapeTestResultAddr(0)
			, m_ShellcodeAddr(0)
			, m_ParamsAddr(0)
			, m_ShellcodeReady(false)
			, m_SpawnVehShellcodeAddr(0)
			, m_SpawnVehParamsAddr(0)
			, m_SpawnVehShellcodeReady(false)
			, CreateVehicleAddr(0)
			, m_safety(false)
		{
			CachedViewMatrix = Matrix4x4();
		}

		void Intialize();
		bool UpdateEntities();
		bool UpdateVehicles();

		HANDLE GetProcHandle() const { return ProcHandle; }
		DWORD GetPid() { return Pid; }
		uint64_t GetModuleBase() { return ModuleBase; };
		uint64_t GetModuleBaseSize() { return ModuleBaseSize; };
		uint64_t GetResquestRagdoll() { return RequestRagdoll; };

		LocalPEDInfo GetLocalPlayerInfo() {
			std::scoped_lock lock(LockLists);
			return LocalPlayerInfo;
		}

		CCamGameplayDirector* GetCamGameplayDirector() { return pCamGameplayDirector; }

		std::vector<Entity> GetEntitiyList() {
			std::scoped_lock lock(LockLists);
			return EntityList;
		}

		std::vector<VehicleInfo> GetVehicleList() {
			std::scoped_lock lock(LockLists2);
			return VehicleList;
		}

		void GetEntitiyListSnapshot(std::vector<Entity>& out) {
			std::scoped_lock lock(LockLists);
			out = EntityList;
		}

		void GetVehicleListSnapshot(std::vector<VehicleInfo>& out) {
			std::scoped_lock lock(LockLists2);
			out = VehicleList;
		}

		uint64_t GetHandleBulletAddress() { return HandleBullet; }
		uint64_t GetCanCombatRollAddress() { return CanCombatRoll; }
		uint64_t GetUpdateCamBasePositionAddress() { return UpdateCamBasePosition; }
		uint64_t GetBlipListAddress() { return BlipList; }

		CPed* GetAimingEntity();
		bool IsPlayerAiming();
		Vector3D GetBonePosVec3(Entity& Ped, unsigned int Mask);
		Vector3D GetBonePosFromCPed(CPed* ped, int boneIndex = 0);
		bool GetPedBoneIndex(Entity& Ped, unsigned int Mask, unsigned int& newIdx);
		void network_request_control_of_entity(uint64_t entity, uint64_t localplayer);

		ImVec2 GetClosestHitBox(Entity Ped);
		bool FindClosestEntity(float Fov, int MaxDistance, bool NPC, bool ClosestFov, Entity* Output);

		void ProcessCameraMovement(Vector3D WorldPosition, int SmoothHorizontal, int SmoothVertical);
		void TeleportToObject(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3D Position, Vector3D VisualPosition, bool Stop);
		void SpectatePed(uint64_t Ped, bool Toggle);

		ImVec2 WorldToScreen(Vector3D Pos);
		ImVec2 WorldToScreen(Vector3D Pos, Matrix4x4 ViewMatrix);
		void UpdateViewMatrix();
		Matrix4x4 GetViewMatrix();
		bool IsOnScreen(ImVec2 Pos);

		std::string GetPlayerName(uint64_t PeerAddress, int GameNetId);
		int GetGameVersion() { return GameVersion; }

		bool IsInitialized() { return bIsIntialized; }

		bool CheckLOS(Vector3D from, Vector3D to);
		bool HasLineOfSight(Vector3D from, Vector3D to);
		void InitRaycastShellcode();
		void InitSpawnVehicleShellcode();
		uint64_t WorldSpawnVehicle(uint32_t modelHash, Vector3D pos, float heading);
		CVehicle* HandleToVehicle(uint64_t handle);

		bool m_safety = false;
		std::unordered_map<int, PedStaticInfo> FriendList;
		std::unordered_map<CPed*, PedStaticInfo> AllEntitesList;
		std::mutex LockLists2;
		HANDLE ProcHandle;

	private:
		bool bIsIntialized = false;

		// Offsets
		uint64_t World;
		uint64_t ReplayInterface;
		uint64_t ViewPort;
		uint64_t Camera;
		uint64_t bIsPlayerAiming;
		uint64_t PlayerAimingAt;
		uint64_t HandleBullet;
		uint64_t GameplayCamHolder;
		uint64_t GameplayCamTarget;
		uint64_t CanCombatRoll;
		uint64_t UpdateCamBasePosition;
		uint64_t BlipList;
		uint64_t NetIdToNamesPtr;
		uint64_t CitizemPlayerNamesModule;
		uint64_t RequestRagdoll;
		uint64_t WorldProbeAddr;
		uint64_t GetShapeTestResultAddr;
		uint64_t CreateVehicleAddr;
		uint64_t m_ShellcodeAddr;
		uint64_t m_ParamsAddr;
		bool m_ShellcodeReady;
		uint64_t m_SpawnVehShellcodeAddr;
		uint64_t m_SpawnVehParamsAddr;
		bool m_SpawnVehShellcodeReady;

		// Process info
		DWORD Pid;
		uint64_t ModuleBase;
		uint64_t ModuleBaseSize;
		std::string ModuleName;
		std::string FivemFolder;
		std::string CrashoMetryLocation;
		std::string ServerIp;
		std::string ServerPort;
		nlohmann::json PlayersInfo;
		std::unordered_map<int, std::string> PlayerIdToName;
		bool LanGame;
		int GameVersion;
		int RealGameVersion;

		// Pointers
		CWorld* pWorld;
		CPed* pLocalPlayer;
		CReplayInterface* pReplayInterface;
		CPedInterface* pPedInterface;
		CVehicleInterface* pVehicleInterface;
		CCamGameplayDirector* pCamGameplayDirector;
		uint64_t pViewPort;
		Matrix4x4 CachedViewMatrix;

		// Lists
		std::mutex LockLists;
		std::vector<Entity> EntityList;
		LocalPEDInfo LocalPlayerInfo;
		std::vector<VehicleInfo> VehicleList;
	};

	inline FivemSDK g_Fivem;
}
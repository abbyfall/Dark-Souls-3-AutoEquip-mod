#include "Core.h"
#include "Hooks.h"
#include "Addresses.h"
#include "ItemAcqHooks.h"
#include <iostream>

DWORD NoWeaponRequirements(UINT_PTR thisPtr, UINT_PTR itemPtr);
DWORD StrRequirements(UINT_PTR thisPtr, UINT_PTR itemPtr);
DWORD AgiRequirements(UINT_PTR thisPtr, UINT_PTR itemPtr);
DWORD IntRequirements(UINT_PTR thisPtr, UINT_PTR itemPtr);
DWORD FthRequirements(UINT_PTR thisPtr, UINT_PTR itemPtr);

DWORD ModCore::Start()
{
	DebugInit();
	if (!Initialize()){
		Panic("Failed to Initialize", "Core\\Core.cpp", 1);
		int3
	};
	return 0;
};

bool ModCore::Initialize()
{
	Addresses::Rebase();

	if (!Settings::LoadSettings("./AutoEquipSettings.ini"))
		Panic("Failed to load 'AutoEquipSettings.ini'", "Core\\Core.cpp", true);

#ifdef DEBUG
	DebugPrint("[AutoEquip] - AutoEquipWeapons = %i", Settings::AutoEquipWeapons);
	DebugPrint("[AutoEquip] - AutoEquipArmor = %i", Settings::AutoEquipArmor);
	DebugPrint("[AutoEquip] - AutoEquipRings = %i", Settings::AutoEquipRings);
	DebugPrint("[AutoEquip] - LeftHandedCatalysts = %i", Settings::LeftHandedCatalysts);
	DebugPrint("[Randomizer] - RandomizeWeaponUpgrade = %i", Settings::RandomWeaponUpgrades);
	DebugPrint("[Randomizer] - RandomInfusionChance = %i", Settings::RandomInfusionChance);
	DebugPrint("[Randomizer] - ReinforceShopWeapons = %i", Settings::ReinforceShopWeapons);
	DebugPrint("[Randomizer] - HighUpgrades = %i", Settings::MoreUpgradedWeapons);
	DebugPrint("[Randomizer] - MaxUpgrades = %i", Settings::MoreUpgradedWeapons);
	DebugPrint("[Misc] - LessWeaponRequirements = %i", Settings::LessWeaponRequirements);
#endif

	if (MH_Initialize() != MH_OK) return false;

	auto ret = Hooks::SetHook(Addresses::GetItemPickup(), &ItemPickupHook, &ItemPickupHookOrig);
	ret &= Hooks::SetHook(Addresses::GetItemBuy(), &ItemBuyHook, &ItemBuyHookOrig);
	
	if (Settings::LessWeaponRequirements != WeaponRequirements::Full) {
		ret &= Hooks::SetHook(Addresses::GetGetStrRequirements(), &StrRequirements);
		ret &= Hooks::SetHook(Addresses::GetGetAgiRequirements(), &AgiRequirements);
		ret &= Hooks::SetHook(Addresses::GetGetIntRequirements(), &IntRequirements);
		ret &= Hooks::SetHook(Addresses::GetGetFthRequirements(), &FthRequirements);
	}

	return ret;
};

void ModCore::Panic(char* message, char* sort, bool isFatalError)
{
	char outmsg[MAX_PATH];
	sprintf_s(outmsg, "[%s] %s", sort, message);

	if (IsDebuggerPresent()) {
		OutputDebugStringA(outmsg);
	};

#ifdef DEBUG
	DebugPrint("ModCore::Panic is outputting debug-mode error information\n");
	DebugPrint(outmsg);
	if (isFatalError) int3;
#else
	auto title = isFatalError ? "[AutoEquip - Fatal Error]" : "[AutoEquip - Error]";
	MessageBoxA(NULL, outmsg, title, MB_ICONERROR);
#endif

	return;
};

void ModCore::DebugInit()
{
#ifdef DEBUG
	FILE* fp;

	AllocConsole();
	SetConsoleTitleA("DS3 AutoEquip Debug Console");
	freopen_s(&fp, "CONOUT$", "w", stdout);
	printf_s("Starting...\n");
#endif
	return;
};

void ModCore::DebugPrint(const char* message, ...)
{
#ifdef DEBUG
	va_list argptr;
	va_start(argptr, message);
	vprintf_s(message, argptr);
	va_end(argptr);
	printf_s("\r\n");
#endif
};

bool Settings::IsAutoEquipEnabled()
{
	return AutoEquipWeapons || AutoEquipArmor || AutoEquipRings;
}

bool Settings::LoadSettings(const std::string& filename)
{
	INIReader reader(filename);
	if (reader.ParseError() == -1) {
		return false;
	};

	AutoEquipWeapons = reader.GetBoolean("AutoEquip", "AutoEquipWeapons", true);
	AutoEquipArmor = reader.GetBoolean("AutoEquip", "AutoEquipArmor", true);
	AutoEquipRings = reader.GetBoolean("AutoEquip", "AutoEquipRings", true);
	LeftHandedCatalysts = reader.GetBoolean("AutoEquip", "LeftHandedCatalysts", true);

	RandomWeaponUpgrades = reader.GetBoolean("Randomizer", "RandomizeWeaponUpgrade", false);
	RandomInfusionChance = reader.GetInteger("Randomizer", "RandomInfusionChance", 0);

	RandomInfusionChance = RandomInfusionChance < 0 ? 0 : RandomInfusionChance;
	RandomInfusionChance = RandomInfusionChance > 100 ? 100 : RandomInfusionChance;

	ReinforceShopWeapons = reader.GetBoolean("Randomizer", "ReinforceShopWeapons", false);
	MoreUpgradedWeapons = reader.GetBoolean("Randomizer", "HighUpgrades", false);
	MaxUpgradedWeapons = reader.GetBoolean("Randomizer", "MaxUpgrades", false);

	LessWeaponRequirements = static_cast<WeaponRequirements>(reader.GetInteger("Misc", "LessWeaponRequirements", 0));
	return true;
}

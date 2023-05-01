#include "script.h"
#include <format>
#include <json.h>
#include <SimpleIni.h>
#include <filesystem>
#include <fstream>
#include <iostream>

using rage__strPackfileManager__AddImageToList = int32_t(*)(const char*, bool, int32_t, bool, uint8_t, bool, bool, bool, bool, bool);
using rage__strPackfileManager__LoadRpf = bool(*)(const char*, unsigned int);
using rage__strPackfileManager__CloseArchive = int64_t(*)(unsigned int);
using rage__strPackfileManager_RemoveUserLock = void(*)(uint32_t);
using rage__strStreamingInfoManager_InvalidateFilesForArchive = void(*)(int64_t* info, uint32_t index, bool);

rage__strPackfileManager__AddImageToList g_AddImageToList;
rage__strPackfileManager__LoadRpf g_LoadRpf;
rage__strPackfileManager__CloseArchive g_CloseArchive;

rage__strPackfileManager_RemoveUserLock g_RemoveUserLock;
rage__strStreamingInfoManager_InvalidateFilesForArchive g_InvalidateFilesForArchive;

int64_t* g_StreamingInfoInstance;

namespace fs = std::filesystem;
using namespace nlohmann;

std::vector<fs::path> g_Rpfs{};
std::vector<json> g_JsonFiles{};

std::vector<std::string> g_RpfNames{};

bool LoadRpf(const char* rpfName)
{
	auto imageIndex = g_AddImageToList(rpfName,
		true, -1, false, 0, false, true, true, false, true);
	if (imageIndex != -1) {
		return g_LoadRpf(rpfName, imageIndex);
	}
	return false;
}

void UnloadRpf(const char* rpfName)
{
	// this will return the image index, if already in list.
	// I know, I could just call FindImage, but I am to lazy
	auto imageIndex = g_AddImageToList(rpfName,
		true, -1, false, 0, false, true, true, false, true);

	if (imageIndex != -1) {
		g_RemoveUserLock(imageIndex);
		g_InvalidateFilesForArchive(g_StreamingInfoInstance, imageIndex, false);
		g_CloseArchive(imageIndex);
	}
}

void TraverseJson(const json& j, std::vector<std::string>& result, std::string path = "") {
	if (j.is_object()) {
		for (const auto& entry : j.items()) {
			std::string new_path = path;
			if (!new_path.empty()) {
				new_path += "/";
			}
			new_path += entry.key();
			TraverseJson(entry.value(), result, new_path);
		}
	}
	else if (j.is_array()) {
		for (const auto& entry : j) {
			TraverseJson(entry, result, path);
		}
	}
	else if (j.is_string()) {
		std::string current_path = path + "/" + j.get<std::string>();
		result.push_back(current_path);
	}
}

void FixFileSystem(fs::path main)
{
	for (const auto& file : fs::directory_iterator(fs::current_path()))
	{
		if (file.is_regular_file()) {
			std::string f = file.path().stem().string();

			if (f.find(".RpfStreamer") != std::string::npos) {
				fs::path dest_path = main / file.path().filename();
				fs::copy_file(file.path(), dest_path, fs::copy_options::overwrite_existing);
				fs::remove(file.path());
			}
		}
	}
}

void LoadJsonFiles()
{
	// Fool proof
	auto mainFolder = fs::current_path() /= "rpfs";

	if (!fs::exists(mainFolder))
		fs::create_directory(mainFolder);

	FixFileSystem(mainFolder);

	for (const auto& file : fs::directory_iterator(mainFolder))
	{
		if (file.is_regular_file()) {
			fs::path extension = file.path().extension();
			if (extension == ".json" && file.path().stem().string().find(".RpfStreamer") != std::string::npos) {
				fs::path rpf = file.path();
				g_Rpfs.push_back(rpf);
			}
		}
	}
}

void AllocateConsole() {
	AllocConsole();
	FILE* stream_in;
	FILE* stream_out;

	SetConsoleTitleA("RpfStreamer");

	freopen_s(&stream_in, "CONIN$", "r", stdin);
	freopen_s(&stream_out, "CONOUT$", "w", stdout);
	freopen_s(&stream_out, "CONOUT$", "w", stderr);
}

void LoadRpfs()
{
	LoadJsonFiles();

	for (const auto& rpf : g_Rpfs) {
		std::ifstream f(rpf.string());

		if (!f.is_open()) {
			std::cerr << "Failed to open " << rpf.string() << std::endl;
			continue;
		}

		json data;
		try {
			data = json::parse(f);
			f.close();
		}
		catch (const json::parse_error& e) {
			std::cerr << "Failed to parse " << rpf.string() << ": " << e.what() << std::endl;
			continue;
		}

		g_JsonFiles.push_back(data);
	}

	if (g_JsonFiles.empty()) {
		std::cerr << "No JSON files found" << std::endl;
	}

	for (const auto& file : g_JsonFiles) {
		TraverseJson(file, g_RpfNames);
	}
}

static int g_HoldReloadKey{};
static int g_ReleaseReloadKey{};

void LoadConfig()
{
	if (!fs::exists("RpfStreamer.ini")) {
		CSimpleIniA ini;
		ini.SetBoolValue("Settings", "ShowConsole", false,
			"If you set this to true a console window will open and show which rpf's have been loaded.");
		ini.SetLongValue("Settings", "HoldReloadKey", VK_CONTROL,
			"Specify the key you want to hold for reloading, key codes can be found here: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes");
		ini.SetLongValue("Settings", "ReleaseReloadKey", VK_F2,
			"Specify the key you want to release for reloading, key codes can be found here: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes");
		ini.SaveFile("RpfStreamer.ini");
	}
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile("RpfStreamer.ini");
	if (ini.GetBoolValue("Settings", "ShowConsole", false))
		AllocateConsole();

	// Default is Ctrl + F2
	g_HoldReloadKey = ini.GetLongValue("Settings", "HoldReloadKey", VK_CONTROL);
	g_ReleaseReloadKey = ini.GetLongValue("Settings", "ReleaseReloadKey", VK_F2);

}

void Magic()
{
	for (const auto& r : g_RpfNames) {
		if (LoadRpf(r.c_str()))
			std::cout << "Loaded " << r << std::endl;
		else
			std::cout << "Failed to load " << r << std::endl;
	}
}

void Reload()
{
	for (const auto& r : g_RpfNames) {
		UnloadRpf(r.c_str());
		if (LoadRpf(r.c_str()))
			std::cout << "Reloaded " << r << std::endl;
		else
			std::cout << "Failed to reload " << r << std::endl;
	}
}

namespace input
{
	const int KEYS_SIZE = 255;
	const int NOW_PERIOD = 100, MAX_DOWN = 5000; // ms

	struct
	{
		std::uint64_t time;
		BOOL isWithAlt;
		BOOL wasDownBefore;
		BOOL isUpNow;
		BOOL curr;
		BOOL prev;
	} keyStates[KEYS_SIZE];

	bool IsWindowFocused()
	{
		auto foregroundHwnd = GetForegroundWindow();
		DWORD foregroundProcId;
		GetWindowThreadProcessId(foregroundHwnd, &foregroundProcId);
		auto currentProcId = GetCurrentProcessId();
		if (foregroundProcId == currentProcId)
		{
			return true;
		}
		return false;
	}

	bool IsKeyDown(DWORD key)
	{
		if (!IsWindowFocused()) return false;
		return (GetAsyncKeyState(key) & 0x8000) != 0;
	}

	bool IsKeyJustUp(DWORD key)
	{
		keyStates[key].curr = IsKeyDown(key);
		if (!keyStates[key].curr && keyStates[key].prev)
		{
			keyStates[key].prev = keyStates[key].curr;
			return true;
		}
		keyStates[key].prev = keyStates[key].curr;
		return false;
	}
}

void Start()
{
	LoadConfig();
	LoadRpfs();

	auto Scanner = scanner(nullptr);
	g_AddImageToList = Scanner.scan("E8 ? ? ? ? 8B D8 85 C0 78 62")
		.Add(1).Rip().As<rage__strPackfileManager__AddImageToList>();
	g_LoadRpf = Scanner.scan("E8 ? ? ? ? 48 89 6C DF")
		.Add(1).Rip().As<rage__strPackfileManager__LoadRpf>();
	g_CloseArchive = Scanner.scan("E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 0C DB")
		.Add(1).Rip().As<rage__strPackfileManager__CloseArchive>();

	g_InvalidateFilesForArchive = Scanner.scan("E8 ? ? ? ? 8B CB E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 0C DB")
		.Add(1).Rip().As<rage__strStreamingInfoManager_InvalidateFilesForArchive>();

	g_RemoveUserLock = Scanner.scan("E8 ? ? ? ? 48 8B 05 ? ? ? ? 83 B8")
		.Add(1).Rip().As<rage__strPackfileManager_RemoveUserLock>();

	g_StreamingInfoInstance = Scanner.scan("48 8D 0D ? ? ? ? 41 03 D0 45 8B C1")
		.Add(3).Rip().As<int64_t*>();

	Magic();
}

void entry()
{
	Start();
	while (true)
	{
		if (input::IsKeyDown(g_HoldReloadKey) && input::IsKeyJustUp(g_ReleaseReloadKey))
			Reload();
		WAIT(0);
	}
}

void ScriptMain()
{	
	entry();
}

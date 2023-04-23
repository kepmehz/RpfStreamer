#include "script.h"
#include <format>
#include <json.h>
#include <SimpleIni.h>
#include <filesystem>
#include <fstream>
#include <iostream>

using rage__strPackfileManager__AddImageToList = int32_t(*)(const char*, bool, int32_t, bool, uint8_t, bool, bool, bool, bool, bool);
using rage__strPackfileManager__LoadRpf = bool(*)(const char*, unsigned int);

rage__strPackfileManager__AddImageToList g_AddImageToList;
rage__strPackfileManager__LoadRpf g_LoadRpf;

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

void LoadConfig()
{
	if (!fs::exists("RpfStreamer.ini")) {
		CSimpleIniA ini;
		ini.SetBoolValue("Settings", "ShowConsole", false,
			"If you set this to true a console window will open and show which rpf's have been loaded.");
		ini.SaveFile("RpfStreamer.ini");
	}
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile("RpfStreamer.ini");
	if (ini.GetBoolValue("Settings", "ShowConsole", false))
		AllocateConsole();
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

	for (const auto& r : g_RpfNames) {
		if (LoadRpf(r.c_str()))
			std::cout << "Loaded " << r << std::endl;
	}
}

void main()
{
	Start();
	while (true)
	{
		WAIT(0);
	}
}

void ScriptMain()
{	
	main();
}

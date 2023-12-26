// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Mod.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Core/File.hpp>
#include <nlohmann/json.hpp>

namespace bw
{
	Mod::Mod(std::filesystem::path modDirectory, ModInfo modInfo) :
	m_modDirectory(std::move(modDirectory)),
	m_info(std::move(modInfo))
	{
		assert(!m_modDirectory.empty());
		m_id = Nz::PathToString(m_modDirectory.filename());
		assert(!m_id.empty());

		// Fetch all assets and scripts files
		if (std::filesystem::path assetDir = m_modDirectory / Nz::Utf8Path("assets"); std::filesystem::is_directory(assetDir))
		{
			for (const std::filesystem::path& path : std::filesystem::recursive_directory_iterator(assetDir))
			{
				if (std::filesystem::is_regular_file(path))
				{
					auto& assetEntry = m_assets.emplace_back();
					assetEntry.assetPath = Nz::PathToString(std::filesystem::relative(path, assetDir));
					assetEntry.physicalPath = path;
				}
			}
		}

		if (std::filesystem::path scriptDir = m_modDirectory / Nz::Utf8Path("scripts"); std::filesystem::is_directory(scriptDir))
		{
			for (const std::filesystem::path& path : std::filesystem::recursive_directory_iterator(scriptDir))
			{
				if (std::filesystem::is_regular_file(path))
				{
					auto& scriptEntry = m_scripts.emplace_back();
					scriptEntry.assetPath = Nz::PathToString(std::filesystem::relative(path, scriptDir));
					scriptEntry.physicalPath = path;
				}
			}
		}
	}

	Mod Mod::LoadFromDirectory(const std::filesystem::path& modDirectory)
	{
		std::vector<Nz::UInt8> content;

		Nz::File infoFile(modDirectory / Nz::Utf8Path("info.json"), Nz::OpenMode::Read);
		if (!infoFile.IsOpen())
			throw std::runtime_error("failed to open info.json file");

		content.resize(infoFile.GetSize());
		if (infoFile.Read(content.data(), content.size()) != content.size())
			throw std::runtime_error("failed to read info.json file");

		nlohmann::json json = nlohmann::json::parse(content.begin(), content.end());

		ModInfo modInfo;
		modInfo.author = json.value("author", "");
		modInfo.description = json.value("description", "");
		modInfo.name = json["name"];

		return Mod(modDirectory, std::move(modInfo));
	}

	std::vector<Mod> Mod::LoadAllFromDirectory(Logger& logger, const std::filesystem::path& modsDirectory)
	{
		std::vector<Mod> mods;

		if (std::filesystem::exists(modsDirectory))
		{
			for (const std::filesystem::path& path : std::filesystem::directory_iterator(modsDirectory))
			{
				try
				{
					mods.emplace_back(LoadFromDirectory(path));
				}
				catch (const std::exception& e)
				{
					bwLog(logger, LogLevel::Error, "failed to load mod info from {0}: {1}", path, e.what());
				}
			}
		}

		return mods;
	}
}

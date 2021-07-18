// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MOD_HPP
#define BURGWAR_CORELIB_MOD_HPP

#include <CoreLib/Export.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace bw
{
	struct ModInfo
	{
		std::string author;
		std::string description;
		std::string name;
	};

	class Logger;

	class BURGWAR_CORELIB_API Mod
	{
		public:
			struct Asset;
			struct Script;

			Mod(std::filesystem::path modDirectory, ModInfo modInfo);
			Mod(const Mod&) = default;
			Mod(Mod&&) noexcept = default;
			~Mod() = default;

			inline const std::vector<Asset>& GetAssets() const;
			inline const std::string& GetId() const;
			inline const ModInfo& GetInfo() const;
			inline const std::filesystem::path& GetPath() const;
			inline const std::string& GetName() const;
			inline const std::vector<Script>& GetScripts() const;

			Mod& operator=(const Mod&) = default;
			Mod& operator=(Mod&&) noexcept = default;

			static Mod LoadFromDirectory(const std::filesystem::path& modDirectory);
			static std::vector<Mod> LoadAllFromDirectory(Logger& logger, const std::filesystem::path& modsDirectory);

			struct Asset
			{
				std::string assetPath;
				std::filesystem::path physicalPath;
			};

			struct Script
			{
				std::string assetPath;
				std::filesystem::path physicalPath;
			};

		private:
			std::filesystem::path m_modDirectory;
			std::string m_id;
			std::vector<Asset> m_assets;
			std::vector<Script> m_scripts;
			ModInfo m_info;
	};
}

#include <CoreLib/Mod.inl>

#endif

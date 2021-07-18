// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Mod.hpp>
#include <cassert>

namespace bw
{
	inline auto Mod::GetAssets() const -> const std::vector<Asset>&
	{
		return m_assets;
	}

	inline const std::string& Mod::GetId() const
	{
		return m_id;
	}

	inline const ModInfo& Mod::GetInfo() const
	{
		return m_info;
	}

	inline const std::filesystem::path& Mod::GetPath() const
	{
		return m_modDirectory;
	}
	
	inline const std::string& Mod::GetName() const
	{
		return m_info.name;
	}
	
	inline auto Mod::GetScripts() const -> const std::vector<Script>&
	{
		return m_scripts;
	}
}

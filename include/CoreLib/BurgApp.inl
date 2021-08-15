// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AnimationStore.hpp>
#include <cassert>
#include <stdexcept>
#include "BurgApp.hpp"

namespace bw
{
	inline Nz::UInt64 BurgApp::GetAppTime() const
	{
		return m_appTime;
	}

	inline const ConfigFile& BurgApp::GetConfig() const
	{
		return m_config;
	}
	
	inline auto BurgApp::GetLogger() -> Logger&
	{
		return m_logger;
	}
	
	inline const tsl::hopscotch_map<std::string, std::shared_ptr<Mod>>& BurgApp::GetMods() const
	{
		return m_mods;
	}

	inline WebService& BurgApp::GetWebService()
	{
		return *m_webService;
	}
}

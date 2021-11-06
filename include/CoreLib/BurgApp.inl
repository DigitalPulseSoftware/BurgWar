// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BurgApp.hpp>
#include <CoreLib/AnimationStore.hpp>
#include <Nazara/Core/Clock.hpp>
#include <cassert>
#include <stdexcept>

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

	inline Nz::UInt64 BurgApp::GetLogTime() const
	{
		return Nz::GetElapsedMicroseconds() - m_startTime;
	}
	
	inline const tsl::hopscotch_map<std::string, std::shared_ptr<Mod>>& BurgApp::GetMods() const
	{
		return m_mods;
	}

	inline WebService& BurgApp::GetWebService()
	{
		assert(m_webService);
		return *m_webService;
	}
}

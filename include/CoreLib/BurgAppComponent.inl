// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BurgAppComponent.hpp>
#include <CoreLib/AnimationStore.hpp>
#include <cassert>
#include <stdexcept>

namespace bw
{
	inline Nz::Time BurgAppComponent::GetAppTime() const
	{
		return m_appTime;
	}

	inline const ConfigFile& BurgAppComponent::GetConfig() const
	{
		return m_config;
	}
	
	inline auto BurgAppComponent::GetLogger() -> Logger&
	{
		return m_logger;
	}

	inline Nz::Time BurgAppComponent::GetLogTime() const
	{
		return Nz::GetElapsedNanoseconds() - m_startTime;
	}
	
	inline const tsl::hopscotch_map<std::string, std::shared_ptr<Mod>>& BurgAppComponent::GetMods() const
	{
		return m_mods;
	}

	inline Nz::WebService& BurgAppComponent::GetWebService()
	{
		assert(m_webService);
		return *m_webService;
	}

	inline bool BurgAppComponent::HasWebService() const
	{
		return m_webService != nullptr;
	}
}

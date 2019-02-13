// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <cassert>

namespace bw
{
	template<typename... Args>
	void SharedGamemode::ExecuteCallback(const std::string& callbackName, Args&&... args)
	{
		sol::protected_function callback = m_gamemodeTable[callbackName];
		if (callback)
		{
			auto result = callback(m_gamemodeTable, std::forward<Args>(args)...);
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << callbackName << " gamemode callback failed: " << err.what() << std::endl;
			}

			//TODO: Handle return
		}
	}

	inline const std::filesystem::path& SharedGamemode::GetGamemodePath() const
	{
		return m_gamemodePath;
	}

	inline sol::table& SharedGamemode::GetGamemodeTable()
	{
		return m_gamemodeTable;
	}

	inline const std::shared_ptr<SharedScriptingContext>& SharedGamemode::GetScriptingContext() const
	{
		return m_context;
	}
}

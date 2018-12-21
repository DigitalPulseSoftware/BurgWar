// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Gamemode.hpp>
#include <cassert>

namespace bw
{
	template<typename... Args>
	void Gamemode::ExecuteCallback(const std::string& callbackName, Args&&... args)
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
}

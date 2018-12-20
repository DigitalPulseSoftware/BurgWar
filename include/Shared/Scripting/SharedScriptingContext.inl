// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <cassert>

namespace bw
{
	inline sol::state& SharedScriptingContext::GetLuaState()
	{
		return m_luaState;
	}

	inline const sol::state& SharedScriptingContext::GetLuaState() const
	{
		return m_luaState;
	}

	inline const std::filesystem::path& bw::SharedScriptingContext::GetCurrentFolder() const
	{
		return m_currentFolder;
	}
}

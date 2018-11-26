// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <cassert>

namespace bw
{
	inline Nz::LuaInstance& SharedScriptingContext::GetLuaInstance()
	{
		return m_luaInstance;
	}

	inline const Nz::LuaInstance& SharedScriptingContext::GetLuaInstance() const
	{
		return m_luaInstance;
	}
}

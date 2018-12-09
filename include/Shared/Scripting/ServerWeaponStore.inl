// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerWeaponStore.hpp>
#include <cassert>

namespace bw
{
	inline ServerWeaponStore::ServerWeaponStore(BurgApp& app, std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context) :
	SharedWeaponStore(std::move(gamemode), std::move(context), true),
	m_application(app)
	{
	}
}

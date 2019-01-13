// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Scripting/ClientGamemode.hpp>
#include <cassert>

namespace bw
{
	inline ClientGamemode::ClientGamemode(LocalMatch& localMatch, std::shared_ptr<SharedScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	SharedGamemode(std::move(scriptingContext), std::move(gamemodePath)),
	m_match(localMatch)
	{
		InitializeGamemode();
	}
}

// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Scripting/ServerGamemode.hpp>
#include <cassert>

namespace bw
{
	inline ServerGamemode::ServerGamemode(Match& localMatch, std::shared_ptr<SharedScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	SharedGamemode(std::move(scriptingContext), std::move(gamemodePath)),
	m_match(localMatch)
	{
		InitializeGamemode();
	}
}

// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/GameState.hpp>

namespace bw
{
	inline const std::shared_ptr<LocalMatch>& GameState::GetMatch()
	{
		return m_match;
	}
}

// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ServerState.hpp>

namespace bw
{
	inline Match& ServerState::GetMatch()
	{
		return *m_match;
	}

	inline const Match& ServerState::GetMatch() const
	{
		return *m_match;
	}
}

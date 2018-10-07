// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Player.hpp>

namespace bw
{
	inline std::size_t Player::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline Match* bw::Player::GetMatch() const
	{
		return m_match;
	}

	inline bool Player::IsInMatch() const
	{
		return m_match != nullptr;
	}
}

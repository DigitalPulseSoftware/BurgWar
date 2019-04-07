// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalMatchComponent.hpp>

namespace bw
{
	inline LocalMatchComponent::LocalMatchComponent(std::shared_ptr<LocalMatch> localMatch) :
	m_localMatch(std::move(localMatch))
	{
	}

	inline const std::shared_ptr<LocalMatch>& LocalMatchComponent::GetLocalMatch() const
	{
		return m_localMatch;
	}
}

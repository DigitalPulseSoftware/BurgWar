// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientAppComponent.hpp>

namespace bw
{
	inline NetworkReactorManager& ClientAppComponent::GetReactorManager()
	{
		return m_networkReactors;
	}
}

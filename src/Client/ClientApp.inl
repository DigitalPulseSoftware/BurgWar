// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApp.hpp>

namespace bw
{
	inline NetworkReactorManager& ClientApp::GetReactorManager()
	{
		return m_networkReactors;
	}
}
// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalInputController.hpp>

namespace bw
{
	inline LocalInputController::LocalInputController(ClientApp& app, Nz::UInt8 localPlayerIndex) :
	m_application(app),
	m_localPlayerIndex(localPlayerIndex)
	{
	}
}
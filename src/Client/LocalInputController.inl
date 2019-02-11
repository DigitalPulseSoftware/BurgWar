// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
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
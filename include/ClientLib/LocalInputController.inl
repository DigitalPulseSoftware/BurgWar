// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalInputController.hpp>

namespace bw
{
	inline LocalInputController::LocalInputController(Nz::UInt8 localPlayerIndex) :
	m_localPlayerIndex(localPlayerIndex)
	{
	}
}
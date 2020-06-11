// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/DummyInputController.hpp>

namespace bw
{
	PlayerInputData& DummyInputController::GetInputs()
	{
		return m_inputs;
	}
	
	PlayerInputData DummyInputController::Poll(LocalMatch& /*localMatch*/, const LocalLayerEntityHandle& /*controlledEntity*/)
	{
		return m_inputs;
	}
}

// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/DummyInputPoller.hpp>

namespace bw
{
	PlayerInputData& DummyInputPoller::GetInputs()
	{
		return m_inputs;
	}
	
	PlayerInputData DummyInputPoller::Poll(LocalMatch& /*localMatch*/, const LocalLayerEntityHandle& /*controlledEntity*/)
	{
		return m_inputs;
	}
}

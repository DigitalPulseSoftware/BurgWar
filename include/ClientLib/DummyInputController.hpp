// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_DUMMYINPUTCONTROLLER_HPP
#define BURGWAR_CLIENTLIB_DUMMYINPUTCONTROLLER_HPP

#include <ClientLib/InputController.hpp>
#include <sol3/forward.hpp>

namespace bw
{
	class DummyInputController : public InputController
	{
		public:
			DummyInputController() = default;
			~DummyInputController() = default;

			PlayerInputData& GetInputs();

			PlayerInputData Poll(LocalMatch& localMatch, const LocalLayerEntityHandle& controlledEntity) override;

		private:
			PlayerInputData m_inputs;
	};
}

#include <ClientLib/DummyInputController.inl>

#endif
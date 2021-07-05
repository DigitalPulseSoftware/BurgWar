// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_DUMMYINPUTPOLLER_HPP
#define BURGWAR_CLIENTLIB_DUMMYINPUTPOLLER_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/InputPoller.hpp>
#include <sol/forward.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API DummyInputPoller : public InputPoller
	{
		public:
			DummyInputPoller() = default;
			~DummyInputPoller() = default;

			PlayerInputData& GetInputs();

			PlayerInputData Poll(ClientMatch& clientMatch, const ClientLayerEntityHandle& controlledEntity) override;

		private:
			PlayerInputData m_inputs;
	};
}

#include <ClientLib/DummyInputPoller.inl>

#endif

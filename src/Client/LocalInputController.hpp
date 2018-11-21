// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALINPUTCONTROLLER_HPP
#define BURGWAR_CLIENT_LOCALINPUTCONTROLLER_HPP

#include <Shared/InputData.hpp>
#include <Nazara/Prerequisites.hpp>

namespace bw
{
	class ClientApp;

	class LocalInputController
	{
		public:
			inline LocalInputController(ClientApp& app, Nz::UInt8 localPlayerIndex);
			~LocalInputController() = default;

			InputData Poll();

		private:
			ClientApp& m_application;
			Nz::UInt8 m_localPlayerIndex;
	};
}

#include <Client/LocalInputController.inl>

#endif
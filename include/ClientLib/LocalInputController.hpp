// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALINPUTCONTROLLER_HPP
#define BURGWAR_CLIENTLIB_LOCALINPUTCONTROLLER_HPP

#include <CoreLib/InputData.hpp>
#include <Nazara/Prerequisites.hpp>

namespace bw
{
	class LocalInputController
	{
		public:
			inline LocalInputController(Nz::UInt8 localPlayerIndex);
			~LocalInputController() = default;

			InputData Poll();

		private:
			Nz::UInt8 m_localPlayerIndex;
	};
}

#include <ClientLib/LocalInputController.inl>

#endif
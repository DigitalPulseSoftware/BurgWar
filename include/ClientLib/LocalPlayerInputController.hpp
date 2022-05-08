// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALPLAYERINPUTCONTROLLER_HPP
#define BURGWAR_CLIENTLIB_LOCALPLAYERINPUTCONTROLLER_HPP

#include <CoreLib/InputController.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API LocalPlayerInputController : public InputController
	{
		public:
			LocalPlayerInputController() = default;
			~LocalPlayerInputController() = default;

			std::optional<PlayerInputData> GenerateInputs(entt::registry& registry, entt::entity entity) const override;
	};
}

#include <ClientLib/LocalPlayerInputController.inl>

#endif

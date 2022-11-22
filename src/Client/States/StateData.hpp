// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_STATEDATA_HPP
#define BURGWAR_STATES_STATEDATA_HPP

#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Widgets/Canvas.hpp>
#include <entt/fwd.hpp>
#include <optional>

namespace bw
{
	class ClientApp;

	struct StateData
	{
		std::optional<Nz::Canvas> canvas;
		ClientApp* app;
		entt::registry* world;
		Nz::RenderWindow* window;
	};
}

#endif

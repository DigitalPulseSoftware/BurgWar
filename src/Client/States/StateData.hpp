// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_STATEDATA_HPP
#define BURGWAR_STATES_STATEDATA_HPP

#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/World.hpp>
#include <optional>

namespace bw
{
	class ClientApp;

	struct StateData
	{
		std::optional<Ndk::Canvas> canvas;
		ClientApp* app;
		Ndk::WorldHandle world;
		Nz::RenderWindow* window;
	};
}

#endif

// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_STATEDATA_HPP
#define BURGWAR_STATES_STATEDATA_HPP

#include <Nazara/Core/EnttWorld.hpp>
#include <Nazara/Renderer/WindowSwapchain.hpp>
#include <Nazara/Platform/Window.hpp>
#include <Nazara/Widgets/Canvas.hpp>
#include <entt/fwd.hpp>
#include <memory>
#include <optional>

namespace Nz
{
	class ApplicationBase;
	class RenderTarget;
}

namespace bw
{
	class ClientAppComponent;

	struct StateData
	{
		std::optional<Nz::Canvas> canvas;
		std::shared_ptr<Nz::RenderTarget> renderTarget;
		ClientAppComponent* appComponent;
		Nz::ApplicationBase* app;
		Nz::EnttWorld* world;
		Nz::Window* window;
		Nz::WindowSwapchain* swapchain;
	};
}

#endif

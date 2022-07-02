// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_DESTRUCTIONWATCHERCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_DESTRUCTIONWATCHERCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <CoreLib/Components/BaseComponent.hpp>
#include <Nazara/Utils/Signal.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API DestructionWatcherComponent : public BaseComponent
	{
		public:
			using BaseComponent::BaseComponent;
			DestructionWatcherComponent(const DestructionWatcherComponent&) = delete;
			DestructionWatcherComponent(DestructionWatcherComponent&&) noexcept = default;
			inline ~DestructionWatcherComponent();

			DestructionWatcherComponent& operator=(const DestructionWatcherComponent&) = delete;
			DestructionWatcherComponent& operator=(DestructionWatcherComponent&&) noexcept = default;

			NazaraSignal(OnDestruction, DestructionWatcherComponent* /*emitter*/);
	};
}

#include <CoreLib/Components/DestructionWatcherComponent.inl>

#endif

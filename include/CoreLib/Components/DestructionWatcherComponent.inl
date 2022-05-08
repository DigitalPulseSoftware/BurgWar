// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/DestructionWatcherComponent.hpp>

namespace bw
{
	inline DestructionWatcherComponent::~DestructionWatcherComponent()
	{
		OnDestruction(this);
	}
}

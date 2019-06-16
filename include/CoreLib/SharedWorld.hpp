// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDWORLD_HPP
#define BURGWAR_CORELIB_SHAREDWORLD_HPP

#include <NDK/World.hpp>

namespace bw
{
	class BurgApp;
	class SharedMatch;

	class SharedWorld
	{
		public:
			SharedWorld(BurgApp& app, SharedMatch& match);
			SharedWorld(const SharedWorld&) = delete;
			SharedWorld(SharedWorld&&) noexcept = default;
			virtual ~SharedWorld();

			Ndk::World& GetWorld();
			const Ndk::World& GetWorld() const;

			SharedWorld& operator=(const SharedWorld&) = delete;
			SharedWorld& operator=(SharedWorld&&) noexcept = default;

		private:
			Ndk::World m_world;
	};
}

#include <CoreLib/SharedWorld.inl>

#endif

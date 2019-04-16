// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDMATCH_HPP
#define BURGWAR_CORELIB_SHAREDMATCH_HPP

#include <CoreLib/TimerManager.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class SharedMatch
	{
		public:
			inline SharedMatch(BurgApp& app);
			SharedMatch(const SharedMatch&) = delete;
			SharedMatch(SharedMatch&&) = delete;
			virtual ~SharedMatch();

			virtual void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) = 0;

			inline TimerManager& GetTimerManager();

			void Update(float elapsedTime);

			SharedMatch& operator=(const SharedMatch&) = delete;
			SharedMatch& operator=(SharedMatch&&) = delete;

		private:
			TimerManager m_timerManager;
	};
}

#include <CoreLib/SharedMatch.inl>

#endif

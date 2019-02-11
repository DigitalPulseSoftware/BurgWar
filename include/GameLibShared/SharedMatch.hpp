// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SHAREDMATCH_HPP
#define BURGWAR_SHARED_SHAREDMATCH_HPP

#include <GameLibShared/TimerManager.hpp>

namespace bw
{
	class SharedMatch
	{
		public:
			inline SharedMatch(BurgApp& app);
			SharedMatch(const SharedMatch&) = delete;
			SharedMatch(SharedMatch&&) = delete;
			virtual ~SharedMatch();

			inline TimerManager& GetTimerManager();

			void Update(float elapsedTime);

			SharedMatch& operator=(const SharedMatch&) = delete;
			SharedMatch& operator=(SharedMatch&&) = delete;

		private:
			TimerManager m_timerManager;
	};
}

#include <GameLibShared/SharedMatch.inl>

#endif

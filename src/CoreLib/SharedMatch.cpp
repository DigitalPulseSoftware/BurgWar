// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/BurgAppComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/LogSystem/EntityLogContext.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <cassert>

namespace bw
{
	namespace
	{
		unsigned int MaxDelayedTick = 10;
	}

	SharedMatch::SharedMatch(BurgAppComponent& app, LogSide side, std::string matchName, Nz::Time tickDuration) :
	m_name(std::move(matchName)),
	m_logger(app, *this, side, app.GetLogger()),
	m_scriptPacketHandler(m_logger),
	m_currentTime(Nz::Time::Zero()),
	m_tickTimer(Nz::Time::Zero()),
	m_tickDuration(tickDuration),
	m_currentTick(0)
	{
		m_logger.SetMinimumLogLevel(LogLevel::Debug);
	}

	SharedMatch::~SharedMatch() = default;

	void SharedMatch::Update(Nz::Time elapsedTime)
	{
		m_tickTimer += elapsedTime;
		std::size_t tickCount = static_cast<Nz::Int64>(m_tickTimer / m_tickDuration);
		if (tickCount > MaxDelayedTick)
		{
			bwLog(m_logger, LogLevel::Warning, "Update is too slow, {} ticks have been discarded to preserve realtime", tickCount - MaxDelayedTick);
			tickCount = MaxDelayedTick;
		}

		for (std::size_t i = 0; i < tickCount; ++i)
		{
			m_tickTimer -= m_tickDuration;
			m_currentTime += m_tickDuration;

			m_timerManager.Update(m_currentTime);

			OnTick(m_tickTimer < m_tickDuration);

			m_currentTick++;
		}
	}
}

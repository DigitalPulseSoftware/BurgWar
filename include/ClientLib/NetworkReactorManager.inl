// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/NetworkReactorManager.hpp>
#include <cassert>

namespace bw
{
	inline NetworkReactorManager::NetworkReactorManager(const Logger& logger) :
	m_logger(logger)
	{
	}

	inline std::size_t NetworkReactorManager::AddReactor(std::unique_ptr<NetworkReactor> reactor)
	{
		m_reactors.emplace_back(std::move(reactor));
		return m_reactors.size() - 1;
	}

	inline void NetworkReactorManager::ClearReactors()
	{
		m_reactors.clear();
	}

	inline const std::unique_ptr<NetworkReactor>& NetworkReactorManager::GetReactor(std::size_t reactorId)
	{
		assert(reactorId < m_reactors.size());
		return m_reactors[reactorId];
	}

	inline std::size_t NetworkReactorManager::GetReactorCount() const
	{
		return m_reactors.size();
	}
}

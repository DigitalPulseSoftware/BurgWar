// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/CancelableState.hpp>

namespace bw
{
	inline const std::shared_ptr<AbstractState>& CancelableState::GetOriginalState()
	{
		return m_originalState;
	}
	
	inline bool CancelableState::IsSwitching() const
	{
		return m_nextStateCallback != nullptr;
	}
}

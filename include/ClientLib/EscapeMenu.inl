// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/EscapeMenu.hpp>

namespace bw
{
	inline void EscapeMenu::Hide()
	{
		return Show(false);
	}

	inline bool EscapeMenu::IsVisible() const
	{
		return m_backgroundWidget->IsVisible();
	}
}

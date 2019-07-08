// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Console.hpp>

namespace bw
{
	inline bool Console::IsVisible() const
	{
		return m_widget->IsVisible();
	}

	inline void Console::Hide()
	{
		return Show(false);
	}
}

// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Console.hpp>

namespace bw
{
	inline void Console::Hide()
	{
		return Show(false);
	}

	inline bool Console::IsVisible() const
	{
		return false;
		//return m_widget->IsVisible();
	}
}

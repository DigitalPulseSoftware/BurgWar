// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Chatbox.hpp>

namespace bw
{
	inline void Chatbox::Close()
	{
		return Open(false);
	}

	inline bool Chatbox::IsOpen() const
	{
		return m_chatEnteringBox->IsVisible();
	}

	inline bool Chatbox::IsTyping() const
	{
		return m_chatEnteringBox->IsVisible();
	}
}

// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/InputComponent.hpp>

namespace bw
{
	inline InputComponent::InputComponent() :
	m_inputIndex(0)
	{
	}

	inline const PlayerInputData& InputComponent::GetInputs() const
	{
		return m_inputData[m_inputIndex];
	}

	inline const PlayerInputData& InputComponent::GetPreviousInputs() const
	{
		std::size_t previousInputIndex = m_inputIndex;
		if (previousInputIndex > 0)
			previousInputIndex--;
		else
			previousInputIndex = m_inputData.size() - 1;

		return m_inputData[previousInputIndex];
	}

	inline void InputComponent::UpdateInputs(const PlayerInputData& inputData)
	{
		if (++m_inputIndex >= m_inputData.size())
			m_inputIndex = 0;

		m_inputData[m_inputIndex] = std::move(inputData);

		OnInputUpdate(this);
	}

	inline void InputComponent::UpdatePreviousInputs(const PlayerInputData& inputData)
	{
		std::size_t previousInputIndex = m_inputIndex;
		if (previousInputIndex > 0)
			previousInputIndex--;
		else
			previousInputIndex = m_inputData.size() - 1;

		m_inputData[previousInputIndex] = inputData;
	}
}

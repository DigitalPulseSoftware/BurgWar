// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/RandomEngine.hpp>

namespace bw
{
	inline RandomEngine::RandomEngine(std::uint_fast64_t seed) :
	m_engine(seed)
	{
	}

	inline std::uint_fast64_t RandomEngine::Generate()
	{
		return m_engine();
	}

	template<typename T>
	T RandomEngine::Generate(T min, T max)
	{
		std::uniform_int_distribution<T> dis(min, max);
		return dis(m_engine);
	}

	inline void RandomEngine::Seed(std::uint_fast64_t seed)
	{
		m_engine.seed(seed);
	}
}

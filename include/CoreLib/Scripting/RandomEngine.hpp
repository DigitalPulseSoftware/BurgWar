// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_RANDOMENGINE_HPP
#define BURGWAR_CORELIB_SCRIPTING_RANDOMENGINE_HPP

#include <CoreLib/Export.hpp>
#include <random>

namespace bw
{
	class BURGWAR_CORELIB_API RandomEngine
	{
		public:
			inline RandomEngine(std::uint_fast64_t seed = std::ranlux48_base::default_seed);
			RandomEngine(const RandomEngine&) = default;
			RandomEngine(RandomEngine&&) = default;
			~RandomEngine() = default;

			inline std::uint_fast64_t Generate();
			template<typename T> T Generate(T min, T max);

			inline void Seed(std::uint_fast64_t seed);

			RandomEngine& operator=(const RandomEngine&) = default;
			RandomEngine& operator=(RandomEngine&&) = default;

		private:
			std::ranlux48_base m_engine;
	};
}

#include <CoreLib/Scripting/RandomEngine.inl>

#endif

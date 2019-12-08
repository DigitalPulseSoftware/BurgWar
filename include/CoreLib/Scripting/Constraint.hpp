// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CONSTRAINT_HPP
#define BURGWAR_CORELIB_SCRIPTING_CONSTRAINT_HPP

#include <Nazara/Physics2D/Constraint2D.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class Constraint
	{
		public:
			inline Constraint(Ndk::EntityHandle entity, Nz::Constraint2DHandle constraint);
			Constraint(const Constraint&) = delete;
			Constraint(Constraint&&) noexcept = default;
			virtual ~Constraint();

			void EnableBodyCollision(bool enable);

			bool IsBodyCollisionEnabled() const;

			void Remove();

			Constraint& operator=(const Constraint&) = delete;
			Constraint& operator=(Constraint&&) noexcept = default;

		protected:
			template<typename T> T* GetConstraint();
			template<typename T> const T* GetConstraint() const;

		private:
			Ndk::EntityHandle m_entity;
			Nz::Constraint2DHandle m_constraint;
	};

	class DampedSpringConstraint : public Constraint
	{
		public:
			inline DampedSpringConstraint(Ndk::EntityHandle entity, Nz::DampedSpringConstraint2D* constraint);
			DampedSpringConstraint(DampedSpringConstraint&&) noexcept = default;
			~DampedSpringConstraint() = default;

			DampedSpringConstraint& operator=(const DampedSpringConstraint&) = delete;
			DampedSpringConstraint& operator=(DampedSpringConstraint&&) noexcept = default;
	};
}

#include <CoreLib/Scripting/Constraint.inl>

#endif

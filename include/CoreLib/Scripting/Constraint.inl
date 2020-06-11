// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/Constraint.hpp>
#include <cassert>

namespace bw
{
	inline bool Constraint::IsValid() const
	{
		return m_constraint.IsValid();
	}

	template<typename T>
	T* Constraint::GetConstraint()
	{
		assert(dynamic_cast<T*>(m_constraint.GetObject()) != nullptr);
		return static_cast<T*>(m_constraint.GetObject());
	}

	template<typename T>
	const T* Constraint::GetConstraint() const
	{
		assert(dynamic_cast<T*>(m_constraint.GetObject()) != nullptr);
		return static_cast<T*>(m_constraint.GetObject());
	}

	inline DampedSpringConstraint::DampedSpringConstraint(Ndk::EntityHandle entity, Nz::DampedSpringConstraint2D* constraint) :
	Constraint(std::move(entity), constraint->CreateHandle())
	{
	}

	inline PinConstraint::PinConstraint(Ndk::EntityHandle entity, Nz::PinConstraint2D* constraint) :
	Constraint(std::move(entity), constraint->CreateHandle())
	{
	}
	
	inline PivotConstraint::PivotConstraint(Ndk::EntityHandle entity, Nz::PivotConstraint2D* constraint) :
	Constraint(std::move(entity), constraint->CreateHandle())
	{
	}

	inline RotaryLimitConstraint::RotaryLimitConstraint(Ndk::EntityHandle entity, Nz::RotaryLimitConstraint2D* constraint) :
	Constraint(std::move(entity), constraint->CreateHandle())
	{
	}
}

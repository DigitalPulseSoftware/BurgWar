// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/Constraint.hpp>
#include <NDK/Components/ConstraintComponent2D.hpp>

namespace bw
{
	Constraint::Constraint(Ndk::EntityHandle entity, Nz::Constraint2DHandle constraint) :
	m_entity(std::move(entity)),
	m_constraint(std::move(constraint))
	{
		m_onDestruction.Connect(m_constraint->OnHandledObjectDestruction, [this](Nz::HandledObject<Nz::Constraint2D>*) {
			KillEntity();
		});
	}

	Constraint::Constraint(Constraint&& constraint) noexcept :
	m_entity(std::move(constraint.m_entity)),
	m_constraint(std::move(constraint.m_constraint))
	{
		constraint.m_onDestruction.Disconnect();
		m_onDestruction.Connect(m_constraint->OnHandledObjectDestruction, [this](Nz::HandledObject<Nz::Constraint2D>*) {
			KillEntity();
		});
	}

	Constraint::~Constraint() = default;
	
	void Constraint::EnableBodyCollision(bool enable)
	{
		AssertValid();
		m_constraint->EnableBodyCollision(enable);
	}

	float Constraint::GetErrorBias() const
	{
		AssertValid();
		return m_constraint->GetErrorBias();
	}

	float Constraint::GetMaxForce() const
	{
		AssertValid();
		return m_constraint->GetMaxForce();
	}
	
	bool Constraint::IsBodyCollisionEnabled() const
	{
		AssertValid();
		return m_constraint->IsBodyCollisionEnabled();
	}
	
	void Constraint::Remove()
	{
		if (!IsValid())
			return;

		if (m_entity && !m_entity->GetComponent<Ndk::ConstraintComponent2D>().RemoveConstraint(m_constraint))
			m_entity->Kill();
	}

	void Constraint::SetErrorBias(float errorBias)
	{
		AssertValid();
		m_constraint->SetErrorBias(errorBias);
	}

	void Constraint::SetMaxForce(float maxForce)
	{
		AssertValid();
		m_constraint->SetMaxForce(maxForce);
	}

	void Constraint::AssertValid() const
	{
		if (!IsValid())
			throw std::runtime_error("Constraint has been removed");
	}

	void Constraint::KillEntity()
	{
		//< FIXME: Deleting entity client-side crashes LocalLayerEntity
		if (m_entity)
			m_entity->Kill();
	}


	float PinConstraint::GetDistance() const
	{
		AssertValid();
		return GetConstraint<Nz::PinConstraint2D>()->GetDistance();
	}

	void PinConstraint::SetDistance(float distance)
	{
		AssertValid();
		GetConstraint<Nz::PinConstraint2D>()->SetDistance(distance);
	}


	Nz::RadianAnglef RotaryLimitConstraint::GetMaxAngle() const
	{
		AssertValid();
		return GetConstraint<Nz::RotaryLimitConstraint2D>()->GetMaxAngle();
	}

	Nz::RadianAnglef RotaryLimitConstraint::GetMinAngle() const
	{
		AssertValid();
		return GetConstraint<Nz::RotaryLimitConstraint2D>()->GetMinAngle();
	}

	void RotaryLimitConstraint::SetMaxAngle(Nz::RadianAnglef maxAngle)
	{
		AssertValid();
		GetConstraint<Nz::RotaryLimitConstraint2D>()->SetMaxAngle(maxAngle);
	}

	void RotaryLimitConstraint::SetMinAngle(Nz::RadianAnglef minAngle)
	{
		AssertValid();
		GetConstraint<Nz::RotaryLimitConstraint2D>()->SetMinAngle(minAngle);
	}
}

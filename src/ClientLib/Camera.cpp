// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Camera.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	Camera::Camera(Ndk::World& world, Nz::RenderTarget* renderTarget, bool perspective) :
	m_isPerspective(perspective)
	{
		m_cameraEntity = world.CreateEntity();

		auto& camera = m_cameraEntity->AddComponent<Ndk::CameraComponent>();
		camera.SetFOV(70.f);
		camera.SetTarget(renderTarget);
		camera.SetZFar(10000.f);

		auto& node = m_cameraEntity->AddComponent<Ndk::NodeComponent>();

		if (perspective)
		{
			camera.SetProjectionScale({ 1.f, -1.f, 1.f });

			m_invFovTan = 1.f / std::tan(Nz::DegreeToRadian(camera.GetFOV() * 0.5f));

			float distance = renderTarget->GetSize().y * 0.5f * m_invFovTan;
			node.SetInitialPosition(renderTarget->GetSize().x * 0.5f, renderTarget->GetSize().y * 0.5f, distance);

			m_projectedDepth = camera.Project({ 0.f, 0.f, 0.f }).z;;
		}
		else
		{
			m_invFovTan = 1.f;
			m_projectedDepth = 0.f;
			camera.SetProjectionType(Nz::ProjectionType_Orthogonal);
		}
	}
	
	Nz::Vector2f Camera::Project(const Nz::Vector2f& worldPosition) const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		return Nz::Vector2f(entityCamera.Project(worldPosition));
	}

	void Camera::MoveToPosition(Nz::Vector2f position)
	{
		position.x = std::floor(position.x);
		position.y = std::floor(position.y);

		OnCameraMove(this, position);

		auto& entityNode = m_cameraEntity->GetComponent<Ndk::NodeComponent>();
		entityNode.SetPosition(position);
	}
	
	Nz::Vector2f Camera::Unproject(const Nz::Vector2f& screenPosition) const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();

		return Nz::Vector2f(entityCamera.Unproject(Nz::Vector3f(screenPosition.x, screenPosition.y, m_projectedDepth)));
	}
}

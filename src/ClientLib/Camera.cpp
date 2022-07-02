// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Camera.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	Camera::Camera(entt::registry& world, Nz::RenderTarget* renderTarget, bool perspective) :
	m_isPerspective(!perspective), // To enable it after
	m_zoomFactor(1.f)
	{
		constexpr float fov = 30.f;

		m_cameraEntity = world.CreateEntity();

		auto& camera = m_cameraEntity->AddComponent<Ndk::CameraComponent>();
		camera.SetFOV(fov);
		camera.SetTarget(renderTarget);
		camera.SetZFar(20000.f);

		m_cameraEntity->AddComponent<Ndk::NodeComponent>();

		m_invFovTan = 1.f / std::tan(Nz::DegreeToRadian(fov * 0.5f));

		EnablePerspective(perspective);

		m_onTargetSizeChanged.Connect(renderTarget->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { UpdateZoomFactor(); });
	}

	void Camera::EnablePerspective(bool enable)
	{
		if (m_isPerspective != enable)
		{
			m_isPerspective = enable;

			UpdateProjection();
		}
	}

	float Camera::GetFOV() const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		return entityCamera.GetFOV();
	}

	Nz::Vector2f Camera::GetPosition() const
	{
		auto& entityNode = m_cameraentity.get<Nz::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition());
	}

	const Nz::Recti& Camera::GetViewport() const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		return entityCamera.GetViewport();
	}

	Nz::Vector2f Camera::Project(const Nz::Vector2f& worldPosition) const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		return Nz::Vector2f(entityCamera.Project(worldPosition));
	}

	Nz::Vector3f Camera::Project(const Nz::Vector3f& worldPosition) const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		return entityCamera.Project(worldPosition);
	}

	void Camera::MoveBy(const Nz::Vector2f& offset)
	{
		auto& entityNode = m_cameraentity.get<Nz::NodeComponent>();
		MoveToPosition(Nz::Vector2f(entityNode.GetPosition()) + offset);
	}

	void Camera::MoveToPosition(Nz::Vector2f position)
	{
		//position.x = std::round(position.x / m_zoomFactor) * m_zoomFactor + 0.5f;
		//position.y = std::round(position.y / m_zoomFactor) * m_zoomFactor + 0.5f;
		position.x = std::floor(position.x);
		position.y = std::floor(position.y);

		OnCameraMove(this, position);

		auto& entityNode = m_cameraentity.get<Nz::NodeComponent>();
		entityNode.SetPosition(position);
	}

	void Camera::SetFOV(float fov)
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		entityCamera.SetFOV(fov);

		m_invFovTan = 1.f / std::tan(Nz::DegreeToRadian(fov * 0.5f));

		if (m_isPerspective)
			UpdateProjection();
	}

	void Camera::SetZoomFactor(float zoomFactor)
	{
		OnCameraZoomFactorUpdate(this, zoomFactor);

		m_zoomFactor = zoomFactor;

		UpdateZoomFactor();
	}

	Nz::Vector2f Camera::Unproject(const Nz::Vector2f& screenPosition) const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();

		return Nz::Vector2f(entityCamera.Unproject(Nz::Vector3f(screenPosition.x, screenPosition.y, m_projectedDepth)));
	}

	Nz::Vector3f Camera::Unproject(const Nz::Vector3f& screenPosition) const
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();

		return entityCamera.Unproject(screenPosition);
	}
	
	void Camera::UpdateProjection()
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();

		if (m_isPerspective)
		{
			entityCamera.SetProjectionScale({ 1.f, -1.f, 1.f });
			entityCamera.SetProjectionType(Nz::ProjectionType_Perspective);
		}
		else
		{
			m_projectedDepth = 0.f;

			entityCamera.SetProjectionScale(Nz::Vector3f::Unit());
			entityCamera.SetProjectionType(Nz::ProjectionType_Orthogonal);
		}

		UpdateZoomFactor();
	}

	void Camera::UpdateZoomFactor()
	{
		auto& entityCamera = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		auto& entityNode = m_cameraentity.get<Nz::NodeComponent>();

		Nz::Vector2f viewportSize = Nz::Vector2f(entityCamera.GetTarget()->GetSize()) / m_zoomFactor;
		//viewportSize.x = std::round(viewportSize.x);
		//viewportSize.y = std::round(viewportSize.y);

		if (m_isPerspective)
		{
			Nz::Vector3f initialPosition;
			initialPosition.x = viewportSize.x * 0.5f;
			initialPosition.y = viewportSize.y * 0.5f;
			initialPosition.z = viewportSize.y * 0.5f * m_invFovTan;
			entityNode.SetInitialPosition(initialPosition);

			m_projectedDepth = entityCamera.Project({ 0.f, 0.f, 0.f }).z;
		}
		else
			entityCamera.SetSize(viewportSize);
	}
}

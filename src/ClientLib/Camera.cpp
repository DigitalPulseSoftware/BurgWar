// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Camera.hpp>
#include <Nazara/Core/EnttWorld.hpp>
#include <Nazara/Graphics/Components/CameraComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	Camera::Camera(Nz::EnttWorld& world, Nz::RenderTarget* renderTarget, bool perspective) :
	m_isPerspective(!perspective), // To enable it after
	m_zoomFactor(1.f)
	{
		constexpr float fov = 30.f;

		m_cameraEntity = world.CreateEntity();

		auto& camera = m_cameraEntity->emplace<Nz::CameraComponent>(renderTarget, Nz::ProjectionType::Orthographic);
		camera.UpdateFOV(fov);
		camera.UpdateTarget(renderTarget);
		camera.UpdateZFar(20000.f);
		camera.UpdateRenderMask(2);

		m_cameraEntity->emplace<Nz::NodeComponent>();

		m_invFovTan = 1.f / std::tan(Nz::DegreeToRadian(fov * 0.5f));

		EnablePerspective(perspective);

		m_onTargetSizeChanged.Connect(renderTarget->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*, const Nz::Vector2ui& newRenderTargetSize) { UpdateZoomFactor(newRenderTargetSize); });
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
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return entityCamera.GetFOV().ToDegrees();
	}

	Nz::Vector2f Camera::GetPosition() const
	{
		auto& entityNode = m_cameraEntity->get<Nz::NodeComponent>();
		return Nz::Vector2f(entityNode.GetPosition());
	}

	const Nz::Recti& Camera::GetViewport() const
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return entityCamera.GetViewport();
	}

	Nz::Vector2f Camera::Project(const Nz::Vector2f& worldPosition) const
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return Nz::Vector2f(entityCamera.Project(worldPosition));
	}

	Nz::Vector3f Camera::Project(const Nz::Vector3f& worldPosition) const
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return entityCamera.Project(worldPosition);
	}

	void Camera::MoveBy(const Nz::Vector2f& offset)
	{
		auto& entityNode = m_cameraEntity->get<Nz::NodeComponent>();
		MoveToPosition(Nz::Vector2f(entityNode.GetPosition()) + offset);
	}

	void Camera::MoveToPosition(Nz::Vector2f position)
	{
		//position.x = std::round(position.x / m_zoomFactor) * m_zoomFactor + 0.5f;
		//position.y = std::round(position.y / m_zoomFactor) * m_zoomFactor + 0.5f;
		position.x = std::floor(position.x);
		position.y = std::floor(position.y);

		OnCameraMove(this, position);

		auto& entityNode = m_cameraEntity->get<Nz::NodeComponent>();
		entityNode.SetPosition(position);
	}

	void Camera::SetFOV(float fov)
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		entityCamera.UpdateFOV(fov);

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
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return Nz::Vector2f(entityCamera.Unproject(Nz::Vector3f(screenPosition.x, screenPosition.y, m_projectedDepth)));
	}

	Nz::Vector3f Camera::Unproject(const Nz::Vector3f& screenPosition) const
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return entityCamera.Unproject(screenPosition);
	}
	
	void Camera::UpdateProjection()
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();

		if (m_isPerspective)
		{
			// EnTT TODO
			//entityCamera.SetProjectionScale({ 1.f, -1.f, 1.f });
			entityCamera.UpdateProjectionType(Nz::ProjectionType::Perspective);
		}
		else
		{
			m_projectedDepth = 0.f;

			// EnTT TODO
			//entityCamera.SetProjectionScale(Nz::Vector3f::Unit());
			entityCamera.UpdateProjectionType(Nz::ProjectionType::Orthographic);
		}

		UpdateZoomFactor();
	}

	void Camera::UpdateZoomFactor()
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		return UpdateZoomFactor(entityCamera.GetRenderTarget().GetSize());
	}

	void Camera::UpdateZoomFactor(const Nz::Vector2ui& newRenderTargetSize)
	{
		auto& entityCamera = m_cameraEntity->get<Nz::CameraComponent>();
		auto& entityNode = m_cameraEntity->get<Nz::NodeComponent>();

		Nz::Vector2f size = Nz::Vector2f(newRenderTargetSize) / m_zoomFactor;
		//viewportSize.x = std::round(viewportSize.x);
		//viewportSize.y = std::round(viewportSize.y);

		if (m_isPerspective)
		{
			Nz::Vector3f initialPosition;
			initialPosition.x = size.x * 0.5f;
			initialPosition.y = size.y * 0.5f;
			initialPosition.z = size.y * 0.5f * m_invFovTan;
			entityNode.SetInitialPosition(initialPosition);

			m_projectedDepth = entityCamera.Project({ 0.f, 0.f, 0.f }).z;
		}
		else
			entityCamera.UpdateSize(size);
	}
}

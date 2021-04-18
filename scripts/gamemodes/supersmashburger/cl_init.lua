include("cl_pcts.lua")

local gamemode = ScriptedGamemode()

function gamemode:UpdateCameraPosition()
	local camera = match.GetCamera()

	local cameraRect
	for _, player in pairs(match.GetPlayers()) do
		local entity = player:GetControlledEntity()
		if (entity) then
			local entityRect = entity:GetGlobalBounds()
			if (cameraRect) then
				cameraRect:ExtendToRect(entityRect)
			else
				cameraRect = entityRect
			end
		end
	end

	local currentLayer = engine_GetActiveLayer()
	if (currentLayer == NoLayer) then
		return
	end

	local cameraZones = match.GetEntitiesByClass("entity_camera_zone", currentLayer)

	local camZone
	if (#cameraZones > 0) then
		camZone = cameraZones[1]:GetRect()
	end

	if (cameraRect) then
		cameraRect = camZone:ComputeIntersection(cameraRect)
		cameraRect:Scale(1.3333)
	else
		if (not camZone) then
			return
		end

		cameraRect = camZone
	end

	local camViewport = camera:GetViewport()
	local zoomFactor = math.min(camViewport.width / cameraRect.width, camViewport.height / cameraRect.height)
	zoomFactor = math.min(zoomFactor, 1.1)

	local scaledViewport = camViewport:GetSize() / zoomFactor

	local offset = Vec2(0.0, 0.0)
	if (cameraRect.width - scaledViewport.x) then
		offset.x = -(scaledViewport.x - cameraRect.width) * 0.5
	end

	if (cameraRect.height - scaledViewport.y) then
		offset.y = -(scaledViewport.y - cameraRect.height) * 0.5
	end

	local camOrigin = cameraRect:GetPosition() + offset
	if (camZone) then
		camOrigin = self:ClampCameraPosition(scaledViewport, camZone, camOrigin)
	end

	camera:MoveToPosition(camOrigin)
	camera:SetZoomFactor(zoomFactor)
end

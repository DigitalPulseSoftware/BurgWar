include("sh_rounds.lua")
include("cl_pcts.lua")

local gamemode = ScriptedGamemode()
gamemode.CameraPos = nil
gamemode.CameraZoom = nil
gamemode.TargetCameraPos = nil
gamemode.TargetCameraZoom = nil

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

	if (camZone and cameraRect) then
		cameraRect = camZone:ComputeIntersection(cameraRect)
		cameraRect:Scale(1.5)
	else
		if (not camZone) then
			return
		end

		cameraRect = camZone
	end

	local camViewport = camera:GetViewport()
	local zoomFactor = math.min(camViewport.width / cameraRect.width, camViewport.height / cameraRect.height)
	zoomFactor = math.min(zoomFactor, 1.0)

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

	self.TargetCameraPos = camOrigin
	self.TargetCameraZoom = zoomFactor

	local frametime = render.GetFrametime()
	if (self.CameraPos) then
		local diff = self.TargetCameraPos - self.CameraPos
		self.CameraPos.x = math.approach(self.CameraPos.x, self.TargetCameraPos.x, diff.x * frametime * 0.9)
		self.CameraPos.y = math.approach(self.CameraPos.y, self.TargetCameraPos.y, diff.y * frametime * 0.9)
	else
		self.CameraPos = self.TargetCameraPos
	end

	if (self.CameraZoom) then
		local diff = self.TargetCameraZoom - self.CameraZoom
		self.CameraZoom = math.approach(self.CameraZoom, self.TargetCameraZoom, diff * frametime * 0.9)
	else
		self.CameraZoom = self.TargetCameraZoom
	end

	camera:MoveToPosition(self.CameraPos)
	camera:SetZoomFactor(self.CameraZoom)
end

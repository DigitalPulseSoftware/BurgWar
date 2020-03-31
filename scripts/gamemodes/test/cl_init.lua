include("cl_scoreboard.lua")

GM.ShakeData = nil

function GM:OnFrame(elapsedTime)
	local playerPosition = engine_GetPlayerPosition(0)
	if (playerPosition) then
		local viewport = engine_GetCameraViewport()
		local cameraOrigin = playerPosition - viewport / 2

		local shakeData = self.ShakeData
		if (shakeData) then
			cameraOrigin = cameraOrigin + Vec2(math.random(-1, 1), math.random(-1, 1)) * shakeData.Strength
			shakeData.Strength = shakeData.Strength - shakeData.StrengthDecrease * elapsedTime
			if (shakeData.Strength < 0) then
				self.ShakeData = nil
			end
		end

		local clampedOrigin
		if (self.CameraRect) then
			local currentRect = self.CameraRect:GetRect()
			local mins = currentRect:GetCorner(false, false)
			local maxs = currentRect:GetCorner(true, true) - viewport

			clampedOrigin = Vec2()
			clampedOrigin.x = math.clamp(cameraOrigin.x, mins.x, maxs.x)
			clampedOrigin.y = math.clamp(cameraOrigin.y, mins.y, maxs.y)

			if (self.NextCameraRect) then
				local targetRect = self.NextCameraRect.rect:GetRect()

				local nextMins = targetRect:GetCorner(false, false)
				local nextMaxs = targetRect:GetCorner(true, true) - viewport

				local elapsedTime = match.GetSeconds() - self.NextCameraRect.time
				local lerpFactor = elapsedTime * 2

				local nextClampedOrigin = Vec2()
				nextClampedOrigin.x = math.clamp(cameraOrigin.x, nextMins.x, nextMaxs.x)
				nextClampedOrigin.y = math.clamp(cameraOrigin.y, nextMins.y, nextMaxs.y)
	
				if (lerpFactor >= 1) then
					clampedOrigin = nextClampedOrigin
					self.CameraRect = self.NextCameraRect.rect
					self.NextCameraRect = nil
				else
					clampedOrigin = math.lerp(clampedOrigin, nextClampedOrigin, lerpFactor)
				end
			end
		end

		engine_SetCameraPosition(clampedOrigin or cameraOrigin)
	end
end

function GM:OnInit()
	self.CameraRect = nil
end

function GM:RefreshCameraRect()
	local playerPosition = engine_GetPlayerPosition(0)
	if (playerPosition) then
		local cameraRects = match.GetEntitiesByClass("entity_camera_rect", engine_GetActiveLayer())

		-- Find most suitable camera rect
		local mostSuitableCameraRect

		for _, cameraRectEntity in pairs(cameraRects) do
			local rect = cameraRectEntity:GetRect()
			if (rect:Contains(playerPosition)) then
				mostSuitableCameraRect = cameraRectEntity
				break
			end
		end

		if (self.CameraRect ~= mostSuitableCameraRect) then
			--if (self.CameraRect and mostSuitableCameraRect) then
			--	if (not self.NextCameraRect or self.NextCameraRect.rect ~= mostSuitableCameraRect) then
			--		self.NextCameraRect = { rect = mostSuitableCameraRect, time = match.GetSeconds() }
			--	end
			--else
				self.CameraRect = mostSuitableCameraRect
			--end
		end
	end
end

GM.OnTick = utils.OverrideFunction(GM.OnTick, function (self)
	self:RefreshCameraRect()
end)

function GM:OnChangeLayer(oldLayer, newLayer)
	-- FIXME: This shouldn't be handled by this callback

	if (oldLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", oldLayer)) do
			ent:OnLeaveLayer(oldLayer)
		end
	end

	if (newLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", newLayer)) do
			ent:OnEnterLayer(newLayer)
		end
	end
end

function GM:ShakeCamera(duration, strength)
	self.ShakeData = {
		Strength = strength,
		StrengthDecrease = strength / duration
	}
end


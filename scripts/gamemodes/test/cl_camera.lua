GM.ShakeData = nil

function GM:ClampCameraPosition(viewport, rect, position)
	local mins = rect:GetCorner(false, false)
	local maxs = rect:GetCorner(true, true) - viewport

	local clampedPos = Vec2()
	clampedPos.x = math.clamp(position.x, mins.x, maxs.x)
	clampedPos.y = math.clamp(position.y, mins.y, maxs.y)

	return clampedPos
end

GM.OnFrame = utils.OverrideFunction(GM.OnFrame, function (self, elapsedTime)
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

		local clampedOrigin = cameraOrigin
		if (self.CameraRect and self.CameraRect:IsValid()) then
			clampedOrigin = self:ClampCameraPosition(viewport, self.CameraRect:GetRect(), cameraOrigin)
		end

		if (self.CameraRectTransition) then
			local transition = self.CameraRectTransition
			local targetRectEntity = transition.entity

			local nextClampedOrigin
			if (targetRectEntity and targetRectEntity:IsValid()) then
				nextClampedOrigin = self:ClampCameraPosition(viewport, targetRectEntity:GetRect(), cameraOrigin)
			else
				nextClampedOrigin = cameraOrigin
			end

			local elapsedTime = match.GetSeconds() - transition.start
			local lerpFactorX = math.min(elapsedTime * transition.lerpFactors.x, 1)
			local lerpFactorY = math.min(elapsedTime * transition.lerpFactors.y, 1)

			if (lerpFactorX >= 1 and lerpFactorY >= 1) then
				clampedOrigin = nextClampedOrigin
				self.CameraRect = transition.entity
				self.CameraRectTransition = nil
			else
				clampedOrigin.x = math.lerp(clampedOrigin.x, nextClampedOrigin.x, lerpFactorX)
				clampedOrigin.y = math.lerp(clampedOrigin.y, nextClampedOrigin.y, lerpFactorY)
			end
		end

		engine_SetCameraPosition(clampedOrigin)
	end
end)

GM.OnInit = utils.OverrideFunction(GM.OnInit, function (self)
	self.CameraRect = nil
end)

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
			local transitionTime
			if (mostSuitableCameraRect and mostSuitableCameraRect:IsValid()) then
				transitionTime = mostSuitableCameraRect:GetTransitionTime()
			else
				transitionTime = Vec2(1, 1)
			end

			if (transitionTime.x > 0 or transitionTime.y > 0) then
				if (not self.CameraRectTransition or self.CameraRectTransition.entity ~= mostSuitableCameraRect) then
					self.CameraRectTransition = { 
						entity = mostSuitableCameraRect,
						lerpFactors = 1.0 / transitionTime,
						start = match.GetSeconds()
					}
				end
			else
				self.CameraRect = mostSuitableCameraRect
				self.CameraRectTransition = nil
			end
		end
	end
end

GM.OnTick = utils.OverrideFunction(GM.OnTick, function (self)
	self:RefreshCameraRect()
end)

function GM:ShakeCamera(duration, strength)
	self.ShakeData = {
		Strength = strength,
		StrengthDecrease = strength / duration
	}
end

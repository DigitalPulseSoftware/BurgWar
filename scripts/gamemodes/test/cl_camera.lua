GM.ShakeData = nil

function GM:ClampCameraPosition(viewportSize, rect, position)
	local mins = rect:GetCorner(false, false)
	local maxs = rect:GetCorner(true, true) - viewportSize

	local clampedPos = Vec2()
	clampedPos.x = math.clamp(position.x, mins.x, maxs.x)
	clampedPos.y = math.clamp(position.y, mins.y, maxs.y)

	return clampedPos
end

function GM:SetCameraScale(scale)
	local camera = match.GetCamera()
	camera:SetZoomFactor(1.0 / scale)
end

GM:On("init", function (self)
	self.CameraRect = nil
end)

GM:On("frame", function (self, elapsedTime)
	local camera = match.GetCamera()

	local playerPosition = engine_GetPlayerPosition(0)
	if (playerPosition) then
		local viewportSize = camera:GetViewport():GetSize() / camera:GetZoomFactor()
		local cameraOrigin = playerPosition - viewportSize / 2

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
			clampedOrigin = self:ClampCameraPosition(viewportSize, self.CameraRect:GetRect(), cameraOrigin)
		end

		if (self.CameraRectTransition) then
			local transition = self.CameraRectTransition
			local targetRectEntity = transition.entity

			local nextClampedOrigin
			if (targetRectEntity and targetRectEntity:IsValid()) then
				nextClampedOrigin = self:ClampCameraPosition(viewportSize, targetRectEntity:GetRect(), cameraOrigin)
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

		camera:MoveToPosition(clampedOrigin)
	else
		self.ShakeData = nil
	end
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

GM:On("tick", function (self)
	self:RefreshCameraRect()
end)

function GM:ShakeCamera(duration, strength)
	self.ShakeData = {
		Strength = strength,
		StrengthDecrease = strength / duration
	}
end

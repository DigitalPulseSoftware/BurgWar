function GM:OnFrame()
	local playerPosition = engine_GetPlayerPosition(0)
	if (playerPosition) then
		local cameraCenter = Vec2(-640, -320) + playerPosition
		cameraCenter.x = math.clamp(cameraCenter.x, 0, 30000)
		cameraCenter.y = math.clamp(cameraCenter.y, -1000, 1664 - engine_GetCameraViewport().y)

		engine_SetCameraPosition(cameraCenter)
	end
end

function GM:OnInit()
end

function GM:OnTick()
end

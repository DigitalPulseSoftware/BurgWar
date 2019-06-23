function GM:OnFrame()
	local playerPosition = engine_GetPlayerPosition(0)
	if (playerPosition) then
		local cameraCenter = Vec2(-640, -320) + playerPosition
		engine_SetCameraPosition(Vec2(math.clamp(cameraCenter.x, 0, 30000), math.clamp(cameraCenter.y, -60000, 3000)))
	end
end

function GM:OnInit()
end

function GM:OnTick()
end

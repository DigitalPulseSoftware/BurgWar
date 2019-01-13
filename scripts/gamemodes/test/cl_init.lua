function GM:OnTick()
	local cameraCenter = Vec2(-640, -320) + engine_GetPlayerPosition()
	engine_SetCameraPosition(Vec2(math.clamp(cameraCenter.x, 0, 3000), math.clamp(cameraCenter.y, -60000, 0)))
end

function GM:OnInit()
end

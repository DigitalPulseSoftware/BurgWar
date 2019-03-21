function GM:OnTick()
	local cameraCenter = Vec2(-640, -320) + engine_GetPlayerPosition(0)
	engine_SetCameraPosition(Vec2(math.clamp(cameraCenter.x, 0, 30000), math.clamp(cameraCenter.y, -60000, 3000)))
end

function GM:OnInit()
end

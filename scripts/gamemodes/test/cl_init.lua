function GM:OnTick()
	engine_SetCameraPosition(Vec2(-640, -320) + engine_GetPlayerPosition())
end

function GM:OnInit()
end

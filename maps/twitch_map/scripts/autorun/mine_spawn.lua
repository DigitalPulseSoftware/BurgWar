if (not SERVER) then
	return
end

local NextSpawnTime = 0

local gamemode = match.GetGamemode()
gamemode:On("Tick", function (self)
	for _, entity in pairs(match.GetEntities()) do
		local pos = entity:GetPosition()
		if (pos.y > 10000) then
			entity:Kill()
		end
	end

	if (match.GetMilliseconds() > NextSpawnTime) then
		match.CreateEntity({
			Type = "entity_clove_mine",
			LayerIndex = 0,
			Position = Vec2(math.random(200, 10000), -700),
			Properties = {
				free = true
			}
		})

		NextSpawnTime = match.GetMilliseconds() + math.random(30, 60) * 1000
	end
end)

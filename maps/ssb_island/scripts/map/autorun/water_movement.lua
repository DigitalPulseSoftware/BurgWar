RegisterClientScript()

if (not CLIENT) then
	return
end

local waterLayer = 0
local frameEvent
local waterEntitiesId = {73,74,75}

local gamemode = match.GetGamemode()
gamemode:On("LayerEnabled", function (self, layerIndex)
	if (layerIndex ~= waterLayer) then
		return
	end

	local waterEntities = {}
	for _, id in pairs(waterEntitiesId) do
		local entity = match.GetEntityByUniqueId(id)
		table.insert(waterEntities, {
			originalPos = entity:GetPosition(),
			entity = entity
		})
	end

	frameEvent = gamemode:On("Frame", function ()
		local t = match.GetSeconds()
		for i, water in pairs(waterEntities) do
			local x = math.sin(t + math.pi / 8 * i) * (50 + (#waterEntities - i) * 50)
			local y = math.cos(t + math.pi / 5 * i) * (0 + (#waterEntities - i) * 3)
			local newPosition = water.originalPos + Vec2(x, y)
			water.entity:SetPosition(newPosition)
		end
	end)
end)

gamemode:On("LayerDisable", function (self, layerIndex)
	if (layerIndex ~= waterLayer) then
		return
	end

	if (frameEvent) then
		gamemode:Disconnect(frameEvent)
		frameEvent = nil
	end
end)

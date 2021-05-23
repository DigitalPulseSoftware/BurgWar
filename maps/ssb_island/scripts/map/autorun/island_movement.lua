RegisterClientScript()

local islandEntityIds = {76, 77, 78, 79, 80}

local gamemode = match.GetGamemode()

local function RegisterEvent()
	for _, id in pairs(islandEntityIds) do
		local entity = match.GetEntityByUniqueId(id)
		entity:SetMomentOfInertia(math.huge)

		local pid = PID(1.5, 0.02, 0.8)
		local originalPosition = entity:GetPosition()
		entity:OverrideMovementController(function (gravity, damping, elapsedTime)
			local currentPos = entity:GetPosition()
			local deltaPos = currentPos - (originalPosition + Vec2(0, math.sin(match.GetSeconds() + math.pi * id / 3)) * 10)
			local newForce = pid:Update(deltaPos, elapsedTime)
			--print(newForce)

			return -newForce, damping
		end)
	end
end

if (SERVER) then
	gamemode:On("MapInit", function()
		RegisterEvent()
	end)
else
	--local islandTick
	local islandLayer = 0

	gamemode:On("LayerEnabled", function (self, layerIndex)
		if (layerIndex ~= islandLayer) then
			return
		end

		RegisterEvent()
	end)

	gamemode:On("LayerDisable", function (self, layerIndex)
		if (layerIndex ~= islandLayer) then
			return
		end

		--islandTick:Disconnect()
		--islandTick = nil
	end)
end

RegisterClientScript()

local gamemode = match.GetGamemode()
local floodEvent

if (SERVER) then
	network.RegisterPacket("SSB_Island_WaterRising")

	local waterState = "low" -- low,rising,high,decreasing
	local nextStateTime = match.GetSeconds() + math.random(3, 9)
	local triggerKillEntity
	local risingDuration
	local waterDecreaseTime
	local floodDuration
	local originalPosition
	local floodOffset

	gamemode:On("Tick", function ()
		if (not triggerKillEntity or not triggerKillEntity:IsValid()) then
			return
		end

		if (waterState == "rising") then
			local pct = 1 - (nextStateTime - match.GetSeconds()) / risingDuration
			triggerKillEntity:SetPosition(originalPosition + floodOffset * math.min(pct, 1))
		elseif (waterState == "decreasing") then
			local pct = (nextStateTime - match.GetSeconds()) / waterDecreaseTime
			triggerKillEntity:SetPosition(originalPosition + floodOffset * math.max(pct, 0))
		end

		local now = match.GetSeconds()
		if (now >= nextStateTime) then
			if (waterState == "low") then
				risingDuration = math.random(10, 20)
				waterDecreaseTime = math.random(5, 20)
				floodDuration = math.random(5, 10)

				waterState = "rising"
				nextStateTime = now + risingDuration

				match.BroadcastChatMessage("La terre tremble...")

				local packet = network.NewPacket("SSB_Island_WaterRising")
				packet:WriteCompressedUnsigned(risingDuration)
				packet:WriteCompressedUnsigned(floodDuration)
				packet:WriteCompressedUnsigned(waterDecreaseTime)
				packet:WriteVector2(floodOffset)

				match.BroadcastPacket(packet)
			elseif (waterState == "rising") then
				waterState = "high"
				nextStateTime = now + floodDuration
			elseif (waterState == "high") then
				waterState = "decreasing"
				nextStateTime = now + waterDecreaseTime
			elseif (waterState == "decreasing") then
				waterState = "low"
				nextStateTime = now + math.random(3, 9)
			end
		end
	end)

	local waterTriggerKill = 67
	gamemode:On("MapInit", function ()
		waterState = "low"
		triggerKillEntity = assert(match.GetEntityByUniqueId(waterTriggerKill))
		nextStateTime = match.GetSeconds() + math.random(3, 9)
		originalPosition = triggerKillEntity:GetPosition()
		floodOffset = Vec2(0, -800)	
	end)
elseif (CLIENT) then
	local waterLayer = 0
	local frameEvent
	local waterEntitiesId = {65, 73,74,75}
	local waterRisingOffset
	local risingDuration
	local floodDuration
	local waterDecreaseTime
	local nextStateTime = 0
	local waterState = "reset"

	network.SetHandler("SSB_Island_WaterRising", function (packet)
		risingDuration = packet:ReadCompressedUnsigned()
		floodDuration = packet:ReadCompressedUnsigned()
		waterDecreaseTime = packet:ReadCompressedUnsigned()
		waterRisingOffset = packet:ReadVector2()

		waterState = "rising"
		nextStateTime = match.GetSeconds() + risingDuration
	end)

	gamemode:On("Tick", function ()
		local now = match.GetSeconds()
		if (now >= nextStateTime) then
			if (waterState == "low") then
				waterState = "rising"
				nextStateTime = now + risingDuration
			elseif (waterState == "rising") then
				waterState = "high"
				nextStateTime = now + floodDuration
			elseif (waterState == "high") then
				waterState = "decreasing"
				nextStateTime = now + waterDecreaseTime
			elseif (waterState == "decreasing") then
				waterState = "reset"
			end
		end
	end)

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

			local waterOffset
			if (waterState == "rising") then
				local pct = 1 - (nextStateTime - t) / risingDuration
				waterOffset = waterRisingOffset * math.min(pct, 1)
			elseif (waterState == "high") then
				waterOffset = waterRisingOffset
			elseif (waterState == "decreasing") then
				local pct = (nextStateTime - t) / waterDecreaseTime
				waterOffset = waterRisingOffset * math.max(pct, 0)
			end

			for i, water in pairs(waterEntities) do
				local x = math.sin(t + math.pi / 8 * i) * (50 + (#waterEntities - i) * 50)
				local y = math.cos(t + math.pi / 5 * i) * (0 + (#waterEntities - i) * 3)
				local newPosition = water.originalPos + Vec2(x, y) + (waterOffset or Vec2())
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

		if (floodEvent) then
			gamemode:Disconnect(floodEvent)
			floodEvent = nil
		end

		waterState = "reset"
	end)
end

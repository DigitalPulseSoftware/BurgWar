RegisterClientScript()

local gamemode = match.GetGamemode()

if (SERVER) then
	network.RegisterPacket("SSB_Island_WaterRising")

	local waterTriggerKill = 67
	gamemode:OnAsync("MapInit", function ()
		local timeBeforeFlood = math.random(30, 90)
		timer.Sleep(timeBeforeFlood * 1000)
		
		match.BroadcastChatMessage("La terre tremble...")

		local risingDuration = math.random(10, 20)
		local waterDecreaseTime = math.random(5, 20)
		local duration = math.random(30, 90)

		local packet = network.NewPacket("SSB_Island_WaterRising")
		packet:WriteCompressedUnsigned(risingDuration)
		packet:WriteCompressedUnsigned(duration)
		packet:WriteCompressedUnsigned(waterDecreaseTime)

		match.BroadcastPacket(packet)

		local triggerKillEntity = assert(match.GetEntityByUniqueId(67))

		local originalPosition = triggerKillEntity:GetPosition()
		local targetPosition = originalPosition - Vec2(0, 800)
		local targetTime = match.GetSeconds() + risingDuration
		local event = gamemode:On("Tick", function ()
			local pct = 1 - (targetTime - match.GetSeconds()) / risingDuration
			triggerKillEntity:SetPosition(math.lerp(originalPosition, targetPosition, math.min(pct, 1))) 
		end)

		timer.Sleep(risingDuration * 1000)
		gamemode:Disconnect(event)
		timer.Sleep(duration * 1000)

		print("Descente des eaux !")

		local targetTime = match.GetSeconds() + waterDecreaseTime
		local event = gamemode:On("Tick", function ()
			local pct = 1 - (targetTime - match.GetSeconds()) / waterDecreaseTime
			triggerKillEntity:SetPosition(math.lerp(targetPosition, originalPosition, math.min(pct, 1))) 
		end)
		timer.Sleep(waterDecreaseTime * 1000)
		gamemode:Disconnect(event)
	end)
elseif (CLIENT) then
	local waterLayer = 0
	local frameEvent
	local waterEntitiesId = {65, 73,74,75}
	local currentRisingOffset = Vec2(0, 0)

	network.SetHandler("SSB_Island_WaterRising", function (packet)
		coroutine.wrap(function()
			local risingDuration = packet:ReadCompressedUnsigned()
			local duration = packet:ReadCompressedUnsigned()
			local waterDecreaseTime = packet:ReadCompressedUnsigned()

			local originalPosition = Vec2(0, 0)
			local targetPosition = originalPosition - Vec2(0, 800)
			local targetTime = match.GetSeconds() + risingDuration
			local event = gamemode:On("Tick", function ()
				local pct = 1 - (targetTime - match.GetSeconds()) / risingDuration
				currentRisingOffset = math.lerp(originalPosition, targetPosition, math.min(pct, 1)) 
			end)

			timer.Sleep(risingDuration * 1000)
			gamemode:Disconnect(event)
			timer.Sleep(duration * 1000)

			local targetTime = match.GetSeconds() + waterDecreaseTime
			local event = gamemode:On("Tick", function ()
				local pct = 1 - (targetTime - match.GetSeconds()) / waterDecreaseTime
				currentRisingOffset = math.lerp(targetPosition, originalPosition, math.min(pct, 1))
			end)
			timer.Sleep(waterDecreaseTime * 1000)
			gamemode:Disconnect(event)
		end)()
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
			for i, water in pairs(waterEntities) do
				local x = math.sin(t + math.pi / 8 * i) * (50 + (#waterEntities - i) * 50)
				local y = math.cos(t + math.pi / 5 * i) * (0 + (#waterEntities - i) * 3)
				local newPosition = water.originalPos + Vec2(x, y) + currentRisingOffset
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
end

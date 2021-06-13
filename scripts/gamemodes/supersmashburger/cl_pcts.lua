
local gamemode = ScriptedGamemode()

gamemode.PlayerPcts = {}
gamemode.PlayerPctTexts = {}

gamemode:On("PlayerControlledEntityUpdate", function (self, player, oldEntity, newEntity)
	local id = player:GetPlayerIndex()

	local text = self.PlayerPctTexts[id]
	if (text and text:IsValid()) then
		text:Hide()
	end

	if (newEntity) then
		local pct = self.PlayerPcts[id] or 0

		local text = newEntity:AddText({
			Hovering = true,
			OutlineThickness = 2,
			RenderOrder = 10,
			Text = pct .. "%"
		})
		text:SetHoveringHeight(50)

		local size = text:GetSize()
		text:SetOffset(Vec2(-size.x / 2, -size.y))

		self.PlayerPctTexts[id] = text
	else
		self.PlayerPctTexts[id] = nil
	end
end)

gamemode:On("PlayerLeave", function (self, player)
	local id = player:GetPlayerIndex()

	local text = self.PlayerPctTexts[id]
	if (text and text:IsValid()) then
		text:Hide()
	end

	self.PlayerPctTexts[id] = nil
	self.PlayerPcts[id] = nil
end)

function gamemode:UpdatePlayerPct(playerId, pct)
	gamemode.PlayerPcts[playerId] = pct
	local text = gamemode.PlayerPctTexts[playerId]
	if (text and text:IsValid()) then
		text:SetText(pct .. "%")

		local size = text:GetSize()
		text:SetOffset(Vec2(-size.x / 2, -size.y))
	end
end

gamemode:On("RoundStateUpdate", function (self, newState)
	if (newState == RoundState.Finished) then
		for _, player in pairs(match.GetPlayers()) do
			local id = player:GetPlayerIndex()
			gamemode:UpdatePlayerPct(id, 0)
		end
	end
end)

network.SetHandler("PctUpdate", function (packet)
	local id = packet:ReadCompressedUnsigned()
	local pct = packet:ReadCompressedUnsigned()

	gamemode:UpdatePlayerPct(id, pct)
end)

local gamemode = ScriptedGamemode()

gamemode.PlayerNicks = {}

gamemode:On("PlayerControlledEntityUpdate", function (self, player, oldEntity, newEntity)
	local id = player:GetPlayerIndex()

	local text = self.PlayerNicks[id]
	if (text) then
		text:Hide()
	end

	if (newEntity) then
		local text = newEntity:AddText({
			Font = "BW_Names",
			Hovering = true,
			OutlineThickness = 2,
			RenderOrder = 10,
			Text = player:GetName()
		})
		text:SetHoveringHeight(20)

		local size = text:GetSize()
		text:SetOffset(Vec2(-size.x / 2, -size.y))

		local team = self:GetPlayerTeam(player)
		if (team) then
			text:SetColor(team:GetColor())
		end

		self.PlayerNicks[id] = text
	else
		self.PlayerNicks[id] = nil
	end
end)

gamemode:On("PlayerNameUpdate", function (self, player, newName)
	local id = player:GetPlayerIndex()
	local text = self.PlayerNicks[id]
	if (text) then
		text:SetText(newName)

		local size = text:GetSize()
		text:SetOffset(Vec2(-size.x / 2, -size.y))
	end
end)

gamemode:On("PlayerLeave", function (self, player)
	local id = player:GetPlayerIndex()

	local text = self.PlayerNicks[id]
	if (text) then
		text:Hide()
	end

	self.PlayerNicks[id] = nil
end)

gamemode:On("PlayerTeamUpdate", function (self, player, team)
	local id = player:GetPlayerIndex()
	local text = self.PlayerNicks[id]
	if (text) then
		text:SetColor(team and team:GetColor() or {r = 255, g = 255, b = 255})
	end
end)

local gamemode = ScriptedGamemode()

gamemode:AddTeam("Brioche", { r = 255, g = 165, b = 0 })
gamemode:AddTeam("Rustic", { r = 166, g = 166, b = 166 })

gamemode:On("PlayerJoined", function (self, player)
	local teams = self:GetTeams()

	-- Find the most empty team
	local bestTeam = 1
	for i = 2, #teams do
		if (teams[i]:GetPlayerCount() < teams[bestTeam]:GetPlayerCount()) then
			bestTeam = i
		end
	end

	local team = teams[bestTeam]
	team:AddPlayer(player)
end)

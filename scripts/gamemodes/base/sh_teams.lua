RegisterClientScript()

local gamemode = ScriptedGamemode()

gamemode.teams = {}
gamemode.teamByName = {}

local teamMt = {}
teamMt.__index = teamMt

function teamMt:GetColor()
	return self.color
end

function teamMt:GetIndex()
	return self.index
end

function teamMt:GetName()
	return self.name
end

function teamMt:GetPlayerIndex(player)
	local playerIndex = player:GetPlayerIndex()
	for i, teamPlayer in pairs(self.players) do
		if (teamPlayer:GetPlayerIndex() == playerIndex) then
			return i
		end
	end
end

function teamMt:GetPlayerCount()
	return #self.players
end

function teamMt:GetPlayers()
	return self.players
end


function gamemode:GetTeams()
	return self.teams
end

function gamemode:GetPlayerTeam(player)
	for _, team in pairs(self:GetTeams()) do
		local index = team:GetPlayerIndex(player)
		if (index) then
			return team, index
		end
	end
end

if (SERVER) then
	function gamemode:AddTeam(name, color)
		assert(self.teamByName[name] == nil, "team already exists")

		local team = setmetatable({
			name = name,
			color = color,
			index = #self.teams + 1,
			players = {}
		}, teamMt)

		table.insert(self.teams, team)
		self.teamByName[name] = team

		match.BroadcastPacket(self:BuildTeamPacket(), false)

		return team
	end

	function teamMt:AddPlayer(player)
		local index = self:GetPlayerIndex(player)
		if (index) then
			return -- Player is already in the team
		end

		table.insert(self.players, player)

		local packet = network.NewPacket("TeamAddPlayer")
		packet:WriteCompressedUnsigned(self.index)
		packet:WriteCompressedUnsigned(player:GetPlayerIndex())

		match.BroadcastPacket(packet, false)
	end

	function teamMt:RemovePlayer(player)
		local index = self:GetPlayerIndex(player)
		if (index) then
			table.remove(self.players, index)

			local packet = network.NewPacket("TeamRemovePlayer")
			packet:WriteCompressedUnsigned(self.index)
			packet:WriteCompressedUnsigned(player:GetPlayerIndex())

			match.BroadcastPacket(packet, false)
		end
	end

	function gamemode:BuildTeamPacket()
		local teams = self:GetTeams()

		local packet = network.NewPacket("TeamData")
		packet:WriteCompressedUnsigned(#teams)
		for _, team in pairs(teams) do
			packet:WriteString(team:GetName())
			packet:WriteColor(team:GetColor())
			local teamPlayers = team:GetPlayers()
			packet:WriteCompressedUnsigned(#teamPlayers)
			for _, player in pairs(teamPlayers) do
				packet:WriteCompressedUnsigned(player:GetPlayerIndex())
			end
		end

		return packet
	end

	gamemode:On("PlayerJoined", function (self, player)
		player:SendPacket(self:BuildTeamPacket())
	end)

	gamemode:On("PlayerLeave", function (self, player)
		for _, team in pairs(self:GetTeams()) do
			team:RemovePlayer(player)
		end
	end)
	
	network.RegisterPacket("TeamData")
	network.RegisterPacket("TeamAddPlayer")
	network.RegisterPacket("TeamRemovePlayer")
else
	network.SetHandler("TeamData", function (packet)
		gamemode.teams = {}
		local teamCount = packet:ReadCompressedUnsigned()
		for i = 1, teamCount do
			local teamName = packet:ReadString()
			local teamColor = packet:ReadColor()

			local team = setmetatable({
				name = teamName,
				color = teamColor,
				index = i,
				players = {}
			}, teamMt)

			table.insert(gamemode.teams, team)
			gamemode.teamByName[teamName] = team
	
			local playerCount = packet:ReadCompressedUnsigned()
			for j = 1, playerCount do
				local playerIndex = packet:ReadCompressedUnsigned()
				table.insert(team.players, match.GetPlayerByIndex(playerIndex))
			end
		end
	end)

	network.SetHandler("TeamAddPlayer", function (packet)
		local teamIndex = packet:ReadCompressedUnsigned()
		local playerIndex = packet:ReadCompressedUnsigned()

		local team = gamemode.teams[teamIndex]
		local player = assert(match.GetPlayerByIndex(playerIndex))

		table.insert(team.players, player)

		match.GetGamemode():Trigger("PlayerTeamUpdate", player, team)
	end)

	network.SetHandler("TeamRemovePlayer", function (packet)
		local teamIndex = packet:ReadCompressedUnsigned()
		local playerIndex = packet:ReadCompressedUnsigned()

		local team = gamemode.teams[teamIndex]
		local player = assert(match.GetPlayerByIndex(playerIndex))

		local index = team:GetPlayerIndex(player)
		assert(index, "player is not part of the team")

		table.remove(team.players, index)

		match.GetGamemode():Trigger("PlayerTeamUpdate", player, nil)
	end)
end

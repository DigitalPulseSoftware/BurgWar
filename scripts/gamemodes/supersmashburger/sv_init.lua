include("sh_scoreboard.lua")
include("sv_rounds.lua")
include("sv_pcts.lua")

local gamemode = ScriptedGamemode()
gamemode.PlayerEntity = "entity_ssb_burger"

gamemode:Disconnect(gamemode.BasePlayerDeathSlot)

gamemode:On("MapInit", function (self)
	local cameraRects = match.GetEntitiesByClass("entity_camera_rect")
	if (#cameraRects > 0) then
		local cameraRect = cameraRects[1]
		local creationInfo = cameraRect:DumpCreationInfo()
		creationInfo.Type = "entity_camera_zone"

		match.CreateEntity(creationInfo)
	end
end)

function gamemode:ChoosePlayerSpawnPosition()
	local spawnpoints = {}

	for _, spawnpoint in pairs(match.GetEntitiesByClass("entity_spawnpoint")) do
		local spawnPos = spawnpoint:GetPosition()
		local spawnLayer = spawnpoint:GetLayerIndex()

		local minDistance = math.huge
		for _, playerEntity in pairs(match.GetEntitiesByClass("entity_ssb_burger", spawnLayer)) do
			minDistance = math.min(minDistance, playerEntity:GetPosition():SquaredDistance(spawnPos))
		end

		table.insert(spawnpoints, {
			position = spawnPos,
			layer = spawnLayer,
			score = minDistance
		})
	end

	table.sort(spawnpoints, function (a, b) return a.score > b.score end)

	local choosedSpawnpoint = spawnpoints[1]
	return choosedSpawnpoint.position, choosedSpawnpoint.layer
end

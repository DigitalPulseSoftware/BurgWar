RegisterClientScript("tilemap.lua")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "mapSize", Type = PropertyType.IntegerSize, Default = Vec2(1, 1), Shared = true },
	{ Name = "cellSize", Type = PropertyType.FloatSize, Default = Vec2(64.0, 64.0), Shared = true },
	{ Name = "content", Type = PropertyType.Integer, Array = true, Default = { 0 }, Shared = true },
	{ Name = "textures", Type = PropertyType.Texture, Array = true, Default = { "" }, Shared = true },
	{ Name = "textureCells", Type = PropertyType.FloatSize, Array = true, Default = { Vec2(1, 1) }, Shared = true },
	{ Name = "mass", Type = PropertyType.Float, Default = 0, Shared = true },
	{ Name = "friction", Type = PropertyType.Float, Default = 1, Shared = true }
}

if (EDITOR) then
	ENTITY.EditorActions = {
		{ 
			Name = "editTilemap",  
			Label = "Edit Tilemap", 
			OnTrigger = function (entityEditor)
				local tileMapEditor = TileMapEditorMode.new(entityEditor:GetTargetEntity(), {
					mapSize = entityEditor:GetProperty("mapSize"),
					tileSize = entityEditor:GetProperty("cellSize"),
					origin = entityEditor:GetEntityPosition(),
					rotation = entityEditor:GetEntityRotation(),
					content = entityEditor:GetProperty("content")
				}, {
					{
						material = "../resources/tiles/1.png",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "../resources/tiles/2.png",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					},
					{
						material = "osef",
						texCoords = Rect(Vec2(0, 0), Vec2(1, 1))
					}
				}, Editor)

				tileMapEditor:SetFinishedCallback(function (tileMapData)
					if (tileMapData) then
						entityEditor:SetEntityPosition(tileMapData.origin)
						entityEditor:SetEntityRotation(tileMapData.rotation)

						entityEditor:SetProperty("mapSize", tileMapData.mapSize)
						entityEditor:SetProperty("cellSize", tileMapData.tileSize)
						entityEditor:SetProperty("content", tileMapData.content)
					end

					entityEditor:Show()
				end)

				Editor:SwitchToMode(tileMapEditor)
				entityEditor:Hide()
			end
		}
	}
end

function ENTITY:Initialize()
	local mapSize = self:GetProperty("mapSize")
	local cellSize = self:GetProperty("cellSize")
	local content = self:GetProperty("content")

	local colliders = {}
	local y = 0

	while (y < mapSize.y) do
		local x = 0
		while (x < mapSize.x) do
			if (content[y * mapSize.x + x + 1] ~= 0) then
				local startX = x
				x = x + 1

				while (x < mapSize.x and content[y * mapSize.x + x + 1] ~= 0) do
					x = x + 1
				end

				local mins = Vec2(startX * cellSize.x, y * cellSize.y)
				local maxs = mins + Vec2((x - startX) * cellSize.x, cellSize.y)

				table.insert(colliders, Rect(mins, maxs))
			end

			x = x + 1
		end

		y = y + 1
	end

	self:SetCollider(colliders)
	self:InitRigidBody(self:GetProperty("mass"), self:GetProperty("friction"))

	if (CLIENT) then
		self:AddTilemap("../resources/dirt.png", mapSize, cellSize, content)
	end
end

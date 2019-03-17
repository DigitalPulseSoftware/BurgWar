RegisterClientScript("tilemap.lua")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {
	{ Name = "width", Type = PropertyType.Integer, Default = 1, Shared = true },
	{ Name = "height", Type = PropertyType.Integer, Default = 1, Shared = true },
	{ Name = "cellWidth", Type = PropertyType.Float, Default = 64.0, Shared = true },
	{ Name = "cellHeight", Type = PropertyType.Float, Default = 64.0, Shared = true },
	{ Name = "content", Type = PropertyType.Integer, Array = true, Default = { 0 }, Shared = true },
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
					mapSize = Vec2(entityEditor:GetProperty("width"), entityEditor:GetProperty("height")),
					tileSize = Vec2(entityEditor:GetProperty("cellWidth"), entityEditor:GetProperty("cellHeight")),
					origin = entityEditor:GetEntityPosition(),
					rotation = entityEditor:GetEntityRotation(),
					content = entityEditor:GetProperty("content")
				}, {
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

						local mapSize = tileMapData.mapSize
						entityEditor:SetProperty("width", mapSize.x)
						entityEditor:SetProperty("height", mapSize.y)

						local tileSize = tileMapData.tileSize
						entityEditor:SetProperty("cellWidth", tileSize.x)
						entityEditor:SetProperty("cellHeight", tileSize.y)

						print(tileMapData.content[0], tileMapData.content[1], tileMapData.content[2])
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
	local width = self:GetProperty("width")
	local height = self:GetProperty("height")
	local cellWidth = self:GetProperty("cellWidth")
	local cellHeight = self:GetProperty("cellHeight")
	local content = self:GetProperty("content")

	local colliders = {}
	local y = 0

	while (y < height) do
		local x = 0
		while (x < width) do
			if (content[y * width + x + 1] ~= 0) then
				local startX = x
				x = x + 1

				while (x < width and content[y * width + x + 1] ~= 0) do
					x = x + 1
				end

				local mins = Vec2(startX * cellWidth, y * cellHeight)
				local maxs = mins + Vec2((x - startX) * cellWidth, cellHeight)

				table.insert(colliders, Rect(mins, maxs))
			end

			x = x + 1
		end

		y = y + 1
	end

	self:SetCollider(colliders)
	self:InitRigidBody(self:GetProperty("mass"), self:GetProperty("friction"))

	if (CLIENT) then
		self:AddTilemap("../resources/dirt.png", Vec2(width, height), Vec2(cellWidth, cellHeight), content)
	end
end

--[[
		std::vector<Nz::Collider2DRef> colliders;
		for (std::size_t y = 0; y < layerData.height; ++y)
		{
			for (std::size_t x = 0; x < layerData.width; ++x)
			{
				if (layerData.tiles[y * layerData.width + x] != 0)
				{
					std::size_t startX = x++;

					while (x < layerData.width && layerData.tiles[y * layerData.width + x] != 0) ++x;

					std::cout << "[client] " << Nz::Rectf(startX * tileMap->GetTileSize().x, y * tileMap->GetTileSize().y, (x - startX) * tileMap->GetTileSize().x, tileMap->GetTileSize().y) << std::endl;
					colliders.emplace_back(Nz::BoxCollider2D::New(Nz::Rectf(startX * tileMap->GetTileSize().x, y * tileMap->GetTileSize().y, (x - startX) * tileMap->GetTileSize().x, tileMap->GetTileSize().y)));
				}
			}
		}

		Nz::CompoundCollider2DRef collider = Nz::CompoundCollider2D::New(std::move(colliders));
		collider->SetCollisionId(0);
]]

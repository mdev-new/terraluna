#include "AssetData.hh"
#include "TileData.hh"
#include <vector>

int main()
{
    std::vector<Assets::TileData> s { 
            Assets::TileData(0, 0, "mud", 0)
    };

    Assets::AssetData assets;
    assets.Pack(
        "",
        s
    );
}
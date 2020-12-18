#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <cstdio>

#include "AssetData.hh"
#include "TileData.hh"

namespace Assets
{
    void AssetData::Pack(std::string path, std::vector<TileData>& data, bool replaceEveryting)
    {
        auto f = std::ofstream("tile_data.tl", std::ios::binary);
        
        for (auto &c : data)
        {
            char* dest = (char*)malloc(30);
            size_t index {};

            int x = 2, y = 3, z = 4;

            memcpy(dest, (char*)&x, sizeof(int));
            index += sizeof(int);

            memcpy(dest + index, "hello world", 12);
            index += 12;

            memcpy(dest + index, (char*)&y, sizeof(int));
            index += sizeof(int);


            memcpy(dest + index, (char*)&z, sizeof(int));
            index += sizeof(int);


            f << dest;
            std::cout << c.id << ' ' << c.name << ' ' << c.x << ' ' << c.y;

        }

        f.flush();
        f.close();

        // auto of = std::ifstream("tile_data.tl", std::ios::binary);

        // while (of)
        // {
        //     TileData _data;

        //     of >> _data.id >> _data.name >> _data.x >> _data.y;
        //     std::cout << _data.id << ' ' << _data.name << ' ' << _data.x << ' ' << _data.y << '\n';
        // }

        // of.close();
    }
} // namespace Assets
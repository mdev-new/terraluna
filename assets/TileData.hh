#pragma once
#include "AssetData.hh"
#include <iostream>

namespace Assets
{
    class TileData
    {
    public:
        // friend std::ofstream &operator>>(std::fstream& is, TileData& td);
        // friend std::ifstream &operator<<(std::fstream &os, TileData &td);
        // friend std::ostream &operator<<(std::ostream &os, TileData &td);

        inline TileData(int x, int y, char name[16], int id)
            : x(x), y(y), id(id)
        {
            this->name = static_cast<char*>(malloc(17));

            strcpy(this->name, name);
        }

        inline TileData()
        {
            this->x = 0;
            this->y = 0;

            this->id = 0;
            
            this->name = reinterpret_cast<char*>(malloc(20));
            
            static int s_Instances;
            s_Instances++;
            

            strcpy(name, "unknown\0");
            std::cout << name << ", ins_created: " << s_Instances << '\n';
        }

    public:
        // Place in the tilemap
        int x, y;

        // Data
        char* name;
        int id;
    };
}
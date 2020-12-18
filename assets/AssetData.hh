#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "TileData.hh"

namespace Assets
{
    class AssetData
    {
    public:
        void Parse(std::string path);
        void Pack(std::string path, std::vector<TileData>& data, bool replaceEveryting = false);

    private:
        TileData m_Data[1024];
    };

    // std::ofstream &operator>>(std::fstream &is, TileData &td);
    // std::ifstream &operator<<(std::fstream &os, TileData &td);
    // std::ostream &operator<<(std::ostream &os, TileData &td);
}
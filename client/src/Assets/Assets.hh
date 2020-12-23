#pragma once
#include <iostream>
#include <vector>
#include "Asset.hh"

class Assets
{
public:
    void Pack(char* data);
    void Parse(const char* texPath, const char* assetPath);

    const Asset& GetAsset(const char* name, const char* tilemap = nullptr) noexcept;

private:
    char* data;

    int x;
    std::vector<Asset> assets;
};
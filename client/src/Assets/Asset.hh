#pragma once
#include "Graphics/Textures/Texture2D.hh"
#include "AssetData.hh"

class Asset
{
public:
    inline const AssetData& GetData() const noexcept
    { return data; }

    inline const Textures::Texture2D& GetTex() const noexcept
    { return tex; }

    inline Asset(const Textures::Texture2D& tex, const AssetData& ass)
        : tex(tex), data(ass)
    {}

private:
    Textures::Texture2D tex;
    AssetData data;
};
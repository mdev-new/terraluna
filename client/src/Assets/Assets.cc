#include "Assets.hh"

void Assets::Pack(char* data)
{
    FILE *write_ptr = fopen("test.bin","wb");  // w for write, b for binary

    fwrite(data, sizeof(data), 1, write_ptr); // write 10 bytes from our buffer
}

void Assets::Parse(const char* texPath, const char* assetPath)
{
    {
        std::string s = texPath;
        Textures::Texture2D tex(s);

        AssetData d;
        
        FILE *f = fopen(assetPath, "rb");
        fread(&d, sizeof(d), 1, f);
        
        Asset ass(tex, d);
        this->assets.emplace_back(ass);

    }
}

const Asset& const 
Assets::GetAsset(
    const char* name, 
    const char* tilemap = nullptr
    ) 
const noexcept
{

}
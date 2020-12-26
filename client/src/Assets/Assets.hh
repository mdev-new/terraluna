/*
 * (c) freestyle 2021
 * Assets: packing, loading etc.
 * Works for shaders, audio, textures and
 * texture data.
 * ACCESS HEREBY GRANTED!
 */


#include <fstream>
#include <string>
#include <iostream>

namespace Assets::Types
{
    enum class ID
    {
        Tex,
        Shader,
        Audio,
        Tilemap,
        Tiledata
    };
    
    struct Texture
    {
        ID id = ID::Tex;
        // TODO: add rest of texture data
    };

    struct Shader
    {
        ID id = ID::Shader;
        const char* code;
    };

    struct Audio
    {
        ID id = ID::Audio;
        // TODO: add rest of audio data
    };

    struct Tilemap
    {
        ID id = ID::Tilemap;
        size_t size;
        char* data;
    };

    struct Tiledata
    {
        ID id = ID::Tiledata;
        uint32_t tilemapId;
        float x, y; // Pos in tilemap
        const char* name;
        uint16_t rarity;
        bool obtainable;
    };

}

namespace Assets::Manager
{
    template<typename T>
    void Pack(const char* fileNameToPack, const T& data)
    {
        auto file = std::fstream(fileNameToPack, std::ios::binary | std::ios::out/* | std::ios::app*/);

        if (file.is_open())
        {
            int header = 1;
            file.write((char*)&header, sizeof(header));
            file.write((char*)&data, sizeof(T));

            file.flush();
            file.close(); 
        }
        else
        {
            std::cout << "FAILED TO OPEN FILE: " << fileNameToPack << '\n';
        }

    }

    template<typename T>
    void Parse(const char* fileNameToParse, T& structure)
    {
        auto file = std::fstream(fileNameToParse, std::ios::binary | std::ios::in);

        if (file.is_open())
        {
            {
                int header; // we dont do anything with the header atleast yet
                file.read((char*)&header, sizeof(header));
            }

            file.read((char*)&structure, sizeof(T));
            file.close();
        }
        else
        {
            std::cout << "FAILED TO OPEN FILE: " << fileNameToParse << '\n';
        }

    }
}
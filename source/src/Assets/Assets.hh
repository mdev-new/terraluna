#pragma once
#include <string>

// FIXME: Error checking
namespace Assets
{
	struct Asset {
		std::string rawdata;
	};

	void Pack(const char *packFilename, const Asset asset)
	{
		int size = sizeof(asset);

		std::fstream file(packFilename, std::ios::binary | std::ios::app);
		file.write((char *)&size, sizeof(int));
		file.write((char *)&asset, sizeof(asset));
		file.flush();
		file.close();
	}

	void Parse(const char *packFilename, int off, Asset& asset)
	{
		std::fstream file(packFilename, std::ios::binary | std::ios::in);

		int sz;

		//file.seekg(off);
		file.read((char *)&sz, sizeof(int));
		file.read((char *)&asset, sz);

		file.close();
	}
}
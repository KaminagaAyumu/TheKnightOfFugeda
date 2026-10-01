#include "JsonReader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cassert>
#include "../MyLib/MyString.h"

using json = nlohmann::json;

namespace
{
	FileManager::FileType ToFileType(const std::string& typeStr)
	{
		if (typeStr == "Image")
		{
			return FileManager::FileType::Image;
		}
		else if (typeStr == "Model")
		{
			return FileManager::FileType::Model;
		}
		else
		{

			std::string hexDump;
			for (unsigned char c : typeStr)
			{
				char buf[4];
				sprintf_s(buf, "%02X ", c);
				hexDump += buf;
			}
			OutputDebugStringA(("JsonReader: 不明なtype値のバイト列 = [" + hexDump + "]\n").c_str());

			assert(false && "JsonReader:不明なリソースの種類です");
			return FileManager::FileType::Image; // デフォルト値を返す
		}
	}
}

std::vector<ResourceRequest> JsonReader::LoadResourceManifest(const std::string& filePath)
{
	std::vector<ResourceRequest> result;

	std::ifstream ifs(filePath);
	if (!ifs.is_open())
	{
		assert(false && "JsonReader:リソースマニフェストファイルが開けません");
		return result;
	}

	json j;
	try
	{
		ifs >> j;
	}
	catch (const json::parse_error& e)
	{
		assert(false && "JsonReader:リソースマニフェストファイルのパースに失敗しました");
		return result;
	}

	if (!j.contains("resources") || !j["resources"].is_array())
	{
		assert(false && "JsonReader:リソースマニフェストファイルに'resources'配列がありません");
		return result;
	}

	for (const auto& elem : j["resources"])
	{
		ResourceRequest req;
		req.path = MyLib::Utf8ToWide(elem.value("path", ""));
		req.type = ToFileType(elem.value("type", "Image"));
		req.isEternal = elem.value("eternal", false);
		result.push_back(req);
	}

	return result;
}

#pragma once
#include <string>
#include <vector>
#include "ResourceRequest.h"

class JsonReader
{
public:

	/// <summary>
	/// 使用するリソースのjsonファイルを読み込む
	/// </summary>
	/// <param name="filePath">jsonファイルのパス</param>
	/// <returns>使用するリソースのリスト</returns>
	static std::vector<ResourceRequest> LoadResourceManifest(const std::string& filePath);
};


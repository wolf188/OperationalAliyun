#include "ConfigManager.h"
#include <memory>
#include <fstream>

CConfigManager * CConfigManager::m_instance = nullptr;


CConfigManager::CConfigManager()
{
	InitConfig();
}

CConfigManager::~CConfigManager()
{

}

void CConfigManager::InitConfig(const std::string &strFilePath)
{
	std::ifstream iFile;

	iFile.open(strFilePath.data(), std::ios::in);
	if (!iFile.is_open())
		return;

	while (!iFile.eof())
	{
		std::string strLine;	
		std::getline(iFile, strLine);
		if (strLine == "")
			continue;
		int nPos = strLine.find_first_of('=');
		const std::string &strKey = StringTrim(strLine.substr(0, nPos), ' ');
		if (strKey[0] == '#') //×¢ÊÍ
			continue;
		const std::string &strVal = StringTrim(strLine.substr(nPos + 1, strLine.length()), ' ');

		if (strKey == "" || strVal == "")
			continue;
		m_mapConfig.emplace(strKey, strVal);
	}
	iFile.close();
}

std::string CConfigManager::GetConfig(const std::string &strkey)
{
	auto it = m_mapConfig.find(strkey);
	if (it != m_mapConfig.end())
	{
		return (*it).second;
	}
	return "";
}

std::string CConfigManager::StringTrim(const std::string &str, char c)
{
	int len = str.length();
	if (len < 1)
	{
		return "";
	}
	int nStart = 0;
	while (nStart < len && str[nStart] == c)
	{
		++nStart;
	}

	int nEnd = str.length() - 1;
	while (nEnd > 0 && str[nEnd] == c)
	{
		--nEnd;
	}

	return str.substr(nStart, nEnd - nStart + 1);
}

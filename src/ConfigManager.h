#ifndef _H_CONFIGMANAGER_H__
#define _H_CONFIGMANAGER_H__

#include "Public.h"
#include <unordered_map>

class CConfigManager:public NoCopy
{
public:
	CConfigManager();
	~CConfigManager();

public:
	static CConfigManager* GetInstance()
	{
		if (!m_instance)
		{
			static CConfigManager _instance;
			m_instance = &_instance;
		}
		return m_instance;
	}

	std::string GetConfig(const std::string &strkey);
	//É¾³ýÇ°ºóÌØÊâ×Ö·û
	std::string StringTrim(const std::string &str, char c = ' ');

private:
	void InitConfig(const std::string &strFilePath = "./config/setting.ini");

private:
	static CConfigManager *m_instance;
	std::unordered_map<std::string, std::string> m_mapConfig;
};

#endif
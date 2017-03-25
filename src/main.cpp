// libCurlTest.cpp : Defines the entry point for the console application.
//

#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <thread>

#include "DomainOperator.h"
#include "ConfigManager.h"
#include "HttpServer.h"

void SplitString(std::vector<std::string> &vec, const std::string &strData, const std::string &Split = ";")
{
	if (strData.length() == 0)
		return;

	int nPos = strData.find_first_of(Split);
	if (-1 != nPos)
	{
		const std::string &strUrl = CConfigManager::GetInstance()->StringTrim(strData.substr(0, nPos));
		vec.emplace_back(strUrl);
		SplitString(vec, strData.substr(nPos + Split.length()), Split);
	}
	else
	{
		const std::string &strUrl = CConfigManager::GetInstance()->StringTrim(strData);
		vec.emplace_back(strData);
	}
}

int main(int argc, char* argv[])
{
	std::cout << R"(
*************************************************
Description:
    动态公网IP绑定阿里云域名！
欢迎入QQ群(565993954)进行技术交流 ^!^
*************************************************)" << std::endl;
	int nInterval = std::atoi(CConfigManager::GetInstance()->GetConfig("interval").data());
	//const std::string &strUrl = CConfigManager::GetInstance()->GetConfig("url");
	const std::string &strDomainName = CConfigManager::GetInstance()->GetConfig("domainName");
	const std::string &strDbName = CConfigManager::GetInstance()->GetConfig("databaseName");

	bool bWhile = true;
	if (strDomainName == "")
	{
		std::cout << "请您在/config/setting.ini中配置绑定的域名！" << std::endl;
		bWhile = false;
	}

	std::vector<std::string> vectorUrl;
	vectorUrl.reserve(10);
	SplitString(vectorUrl, strDomainName);

	while (true && bWhile)
	{
		for (auto &val : vectorUrl)
		{
			CDomainOperator::GetInstance()->UpdateDomainRecord("http://alidns.aliyuncs.com/", val);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (nInterval>10*1000 && nInterval < 12*3600*1000)
			std::this_thread::sleep_for(std::chrono::milliseconds(nInterval));
		else
			std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	
	getchar();
	return 0;
}


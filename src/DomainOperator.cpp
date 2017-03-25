#include "DomainOperator.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <thread>
#include <iostream>

#include "DomainManager.h"
#include "HttpServer.h"

#ifdef WIN32
#include <windows.h>
#endif

CDomainOperator * CDomainOperator::m_instance = nullptr;
using namespace rapidjson;

CDomainOperator::CDomainOperator()
{

}

CDomainOperator::~CDomainOperator()
{

}

void CDomainOperator::UpdateDomainRecord(const std::string &strUrl, const std::string &strDomainName)
{
	m_strAddress = CHttpServer::GetInstance()->GetPublicIpAddress();

	if (m_strAddress.empty())
		return;
	
	const std::string &strRecords = CDomainManager::GetInstance()->GetDomainRecords(strUrl, strDomainName);
	
	std::cout<<"m_strAddress = "<<m_strAddress<<std::endl;
	std::cout<<"strRecords = "<<strRecords;
	Document doc;
	doc.Parse(strRecords.data());
	if (!doc.HasMember("DomainRecords"))
		return;

	Value &varDomain = doc["DomainRecords"];
	if (!varDomain.IsObject() || !varDomain.HasMember("Record"))
		return;

	rapidjson::Value &var = doc["DomainRecords"]["Record"];

	if (var.IsArray())
	{
		int nTime = 0;
		int s = var.Size();
		for (int i = 0; i < s; ++i)
		{
			const rapidjson::Value& ob = var[i];
			if (!ob.IsObject())
				continue;
			if (!ob.HasMember("RR") || !ob.HasMember("RecordId") 
				|| !ob.HasMember("Type") || !ob.HasMember("Value"))
				continue;
				
			std::string strRR = ob["RR"].GetString();
			if (strRR == "@")
				strRR = CHttpServer::GetInstance()->UrlEncode(strRR);
			const std::string &strRecordId = ob["RecordId"].GetString();
			const std::string &strType = ob["Type"].GetString();
			const std::string &strValue = ob["Value"].GetString();

			if (m_strAddress != strValue)
			{
				const std::string &response = CDomainManager::GetInstance()->UpdateDomainRecords(strUrl, strRecordId, m_strAddress, strRR, strType);
				std::cout << response.data() << std::endl;

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	}
}

#ifndef _H_DOMAINMANAGER_H__
#define _H_DOMAINMANAGER_H__

#include "Public.h"
#include <map>

class CDomainManager
{
public:
	explicit CDomainManager(const std::string &strFormat = "JSON");
	~CDomainManager();

public:
	static CDomainManager* GetInstance()
	{
		if (!m_instance)
		{
			static CDomainManager _instance;
			m_instance = &_instance;
		}
		return m_instance;
	}

	void PushRequestParameters(const std::string &strKey, const std::string &strVal);
	void ClearParameters();
	//获取解析记录列表
	std::string GetDomainRecords(const std::string &strUrl, const std::string &strDomainName);
	std::string UpdateDomainRecords(const std::string &strUrl,
		const std::string &strRecordId,
		const std::string &strValue,
		const std::string &strRR,
		const std::string &strType = "A");
	inline void SetResultFormat(const std::string &format)
	{
		m_strFormat = format;
	}

private:
	void SetPublicParameter();
	std::string GetUrl(const std::string &strUrl);
	std::string GetHttpInfo(const std::string &strUrl);

private:
	static CDomainManager *m_instance;
	char m_accessKeyId[32];
	char m_accessSecret[32];
	std::string m_strFormat = "JSON";
	std::map<std::string, std::string> m_requestParameters;
};

#endif

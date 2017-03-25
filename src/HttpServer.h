#ifndef _H_HTTPSERVER_H__
#define _H_HTTPSERVER_H__

#include "Public.h"
#include <string>

class CHttpServer: public NoCopy
{
public:
	CHttpServer();
	~CHttpServer();

public:
	static CHttpServer *GetInstance()
	{
		if (!m_instance)
		{
			static CHttpServer _instance;
			m_instance = &_instance;
		}
		return m_instance;
	}

	std::string GetHttpInfo(const std::string &strUrl);
	std::string GetPublicIpAddress();
	std::string UrlEncode(const std::string& szToEncode);
	std::string UrlDecode(const std::string& szToDecode);

private:
	static CHttpServer *m_instance;
};

#endif
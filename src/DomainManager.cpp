#include "DomainManager.h"
#include <time.h>
#include <string.h>
#include <iostream>
#include <memory>

#include "HttpServer.h"
#include "hmac-sha1.h"
#include "base64.h"
#include "ConfigManager.h"

const char *g_szDomain = "http://alidns.aliyuncs.com/";		//获取域名列表
const char *g_szInstances = "http://ecs.aliyuncs.com/";		//查询实例列表,实例状态

CDomainManager * CDomainManager::m_instance = nullptr;

CDomainManager::CDomainManager(const std::string &strFormat)
{
	const std::string &strAccessId = CConfigManager::GetInstance()->GetConfig("accessKeyId");
	const std::string &strAccessSecret = CConfigManager::GetInstance()->GetConfig("accessSecret");
	memset(m_accessKeyId, 0, sizeof(m_accessKeyId));
	strcpy(m_accessKeyId, strAccessId.data());
	memset(m_accessSecret, 0, sizeof(m_accessSecret));
	strcpy(m_accessSecret, strAccessSecret.data());
}

CDomainManager::~CDomainManager()
{

}

void CDomainManager::SetPublicParameter()
{
	m_requestParameters.clear();
	//返回值的类型，支持JSON与XML。默认为XML
	m_requestParameters.emplace("Format", m_strFormat);	

	//API版本号，为日期形式：YYYY-MM-DD，本版本对应为2015-01-09
	m_requestParameters.emplace("Version", "2015-01-09");

	//阿里云颁发给用户的访问服务所用的密钥ID
	m_requestParameters.emplace("AccessKeyId", m_accessKeyId);

	//签名方式，目前支持HMAC - SHA1
	m_requestParameters.emplace("SignatureMethod", "HMAC-SHA1");

	//签名算法版本，目前版本是1.0
	m_requestParameters.emplace("SignatureVersion", "1.0");

	/*请求的时间戳。日期格式按照ISO8601标准表示，并需要使用UTC时间。格式为YYYY-MM-DDThh:mm:ssZ 
	例如，2015-01-09T12:00:00Z（为UTC时间2015年1月9日12点0分0秒）*/
	time_t t = time(nullptr);
	struct tm *_tm = gmtime(&t);
	char szTimeStamp[32] = { 0 };
#ifdef WIN32
	_snprintf(szTimeStamp, sizeof(szTimeStamp)-1, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		_tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
#else
	snprintf(szTimeStamp, sizeof(szTimeStamp)-1, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		_tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);

#endif
	m_requestParameters.emplace("Timestamp", CHttpServer::GetInstance()->UrlEncode(szTimeStamp));

	//唯一随机数，用于防止网络重放攻击。用户在不同请求间要使用不同的随机数值
	srand(time(NULL));
	static unsigned int nRand = 0;
	const std::string &strSignatureNonce = std::to_string(t) + std::to_string(nRand++) + std::to_string(rand()%10000000);
	m_requestParameters.emplace("SignatureNonce", strSignatureNonce);
}

void CDomainManager::ClearParameters()
{
	m_requestParameters.clear();
}

std::string CDomainManager::GetDomainRecords(const std::string &strUrl, const std::string &strDomainName)
{	
	SetPublicParameter();

	//获取域名列表
	m_requestParameters.emplace("Action", "DescribeDomainRecords");
	m_requestParameters.emplace("DomainName", strDomainName);

	return GetHttpInfo(strUrl);
}

std::string CDomainManager::GetHttpInfo(const std::string &strUrl)
{
	std::string strToSig = "GET&%2F&";
	std::string strRequest;
	int nIndex = 0;
	for (auto &val : m_requestParameters)
	{
		if (nIndex != 0)	//第一次不加&号
			strRequest += "&";
		strRequest += val.first;
#if _DEBUG
		std::cout << val.first.data() << " = " << val.second.data() << std::endl;
#endif
		strRequest += "=";
		strRequest += val.second;
		++nIndex;
	}
	strRequest = std::move(CHttpServer::GetInstance()->UrlEncode(strRequest));
	strToSig += strRequest;
	unsigned char szHmacSha[128] = { 0 };
	unsigned char tmpKey[32] = { 0 };

	strcpy((char*)tmpKey, m_accessSecret);
	tmpKey[strlen((char*)tmpKey)] = '&';
#if _DEBUG
	std::cout << "strToSig = " << strToSig.data() << std::endl;
#endif
#ifdef WIN32
	hmac_sha(tmpKey, strlen((char*)tmpKey), (unsigned char*)strToSig.data(), strToSig.length(), szHmacSha, 20);
	const std::string &strSig = base64_encode(szHmacSha, strlen((char*)szHmacSha));
#else
	char szCmd[2048] = {0};

	snprintf(szCmd, sizeof(szCmd)-1, R"(echo -n "%s" |openssl sha1 -binary -hmac "%s" |base64)", strToSig.data(), tmpKey);
	std::cout<<"szCmd = "<<szCmd<<std::endl;
	FILE* pFile = popen(szCmd, "r");
	if(!pFile)
		return "";
	char *tmpSig = new char[64]();
	std::shared_ptr<void> p(tmpSig, [&](void *){
					if(tmpSig)
					{
						delete[] tmpSig;
					}
					if(pFile)
					{
						fclose(pFile);
					}
					});
	size_t len = 64;	
	getline(&tmpSig, &len, pFile);
	tmpSig[strlen(tmpSig)-1] = '\0';
	const std::string &strSig = tmpSig;
#endif
	std::cout << "Signature = " << strSig.data() << std::endl;
	const std::string &strSigEncode = CHttpServer::GetInstance()->UrlEncode(strSig);

	std::cout << "Signature = " << strSigEncode.data() << std::endl;

	m_requestParameters.emplace("Signature", strSigEncode);

	strRequest = "";
	nIndex = 0;
	for (auto &val : m_requestParameters)
	{
		if (nIndex != 0)	//第一次不加&号
			strRequest += "&";
		strRequest += val.first;
		strRequest += "=";
		strRequest += val.second;
		++nIndex;
	}

	strRequest = GetUrl(strUrl) + strRequest;
	return CHttpServer::GetInstance()->GetHttpInfo(strRequest);
}

std::string CDomainManager::UpdateDomainRecords(const std::string &strUrl, 
	const std::string &strRecordId, 
	const std::string &strValue, 
	const std::string &strRR, 
	const std::string &strType /*= "A"*/)
{
	SetPublicParameter();

	m_requestParameters.emplace("Action", "UpdateDomainRecord");
	m_requestParameters.emplace("RecordId", strRecordId);
	m_requestParameters.emplace("RR", strRR);
	m_requestParameters.emplace("Type", strType);
	m_requestParameters.emplace("Value", strValue);

	return GetHttpInfo(strUrl);
}

std::string CDomainManager::GetUrl(const std::string &strUrl)
{
	std::string strTmpUrl = strUrl;
	if (strTmpUrl[strTmpUrl.length()-1] == '/')
	{
		strTmpUrl.append("?");
	}
	else
	{
		strTmpUrl.append("/?");
	}
	return strTmpUrl;
}



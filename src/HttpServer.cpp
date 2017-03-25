#include "HttpServer.h"
#include "curl/curl.h"
#include <iostream>
#include <string.h>

CHttpServer * CHttpServer::m_instance = nullptr;

static size_t read_callback(void *ptr, size_t size, size_t nmemb, std::string &stream)
{
	/* in real-world cases, this would probably get this data differently
	as this fread() stuff is exactly what the library already would do
	by default internally */
	stream += std::string((char*)ptr, size*nmemb);

	return size*nmemb;
}

CHttpServer::CHttpServer()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

CHttpServer::~CHttpServer()
{
	curl_global_cleanup();
}

std::string CHttpServer::GetHttpInfo(const std::string &strUrl)
{
	CURL *curl = curl_easy_init();
	if (!curl)
		return "";

	std::string strBuff;

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.data());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strBuff);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_callback);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cout << curl_easy_strerror(res)<< std::endl;
		return "";
	}

	return strBuff;
}

std::string CHttpServer::GetPublicIpAddress()
{
	//提取本机公网IP
	const std::string &strAddress = GetHttpInfo("http://1212.ip138.com/ic.asp");
	
	int pos = strAddress.find("您的IP是：[");
	pos += strlen("您的IP是：[");
	int pos2 = strAddress.find("] 来自");
	if (pos < 0 || pos2 < 0 || pos2 - pos < 10)
		return "";
	return strAddress.substr(pos, pos2 - pos);
}

std::string CHttpServer::UrlEncode(const std::string& szToEncode)
{
	std::string src = szToEncode;
	char hex[] = "0123456789ABCDEF";
	std::string dst;
	
	for (size_t i = 0; i < src.size(); ++i)
	{
		unsigned char cc = src[i];
		if (cc >= 'A' && cc <= 'Z'
			|| cc >= 'a' && cc <= 'z'
			|| cc >= '0' && cc <= '9'
			|| cc == '.'
			|| cc == '_'
			|| cc == '-'
			|| cc == '~')
		{
			if (cc == ' ')
			{
				dst += "+";
			}
			else
				dst += cc;
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return dst;
}

std::string CHttpServer::UrlDecode(const std::string& szToDecode)
{
	std::string result;
	int hex = 0;
	for (size_t i = 0; i < szToDecode.length(); ++i)
	{
		switch (szToDecode[i])
		{
		case '+':
			result += ' ';
			break;
		case '%':
			if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
			{
				std::string hexStr = szToDecode.substr(i + 1, 2);
				hex = strtol(hexStr.c_str(), 0, 16);
				//字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]  
				//可以不经过编码直接用于URL  
				if (!((hex >= 48 && hex <= 57) || //0-9  
					(hex >= 97 && hex <= 122) ||   //a-z  
					(hex >= 65 && hex <= 90) ||    //A-Z  
					//一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]  
					hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
					|| hex == 0x2a || hex == 0x2b || hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
					|| hex == 0x3A || hex == 0x3B || hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
					))
				{
					result += char(hex);
					i += 2;
				}
				else result += '%';
			}
			else {
				result += '%';
			}
			break;
		default:
			result += szToDecode[i];
			break;
		}
	}
	return result;
}

#ifndef _H_DOMAINOPERATOR_H__
#define _H_DOMAINOPERATOR_H__

#include "Public.h"

class CDomainOperator: public NoCopy
{
public:
	CDomainOperator();
	~CDomainOperator();

public:
	static CDomainOperator* GetInstance()
	{
		if (!m_instance)
		{
			static CDomainOperator _instance;
			m_instance = &_instance;
		}
		return m_instance;
	}

	void UpdateDomainRecord(const std::string &strUrl, const std::string &strDomainName);
	
private:
	static CDomainOperator *m_instance;
	std::string				m_strAddress;
};

#endif
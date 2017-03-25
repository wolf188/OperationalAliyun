#ifndef _H_PUBLIC_H__
#define _H_PUBLIC_H__

#include <string>
#include <iostream>

class NoCopy
{
protected:
	NoCopy(){};
	NoCopy(const NoCopy&);
	NoCopy& operator=(const NoCopy&);
	~NoCopy(){};
};


#endif
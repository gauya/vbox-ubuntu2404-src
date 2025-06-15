#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef __GSTR_H__
#define __GSTR_H__

#ifdef __cplusplus
extern "C" {
#include <parse.h>
#endif


#ifdef __cplusplus
}

class gstr {
	char *_str;
	uint32_t _len;
	uint32_t _bsize;
public:
	gstr(const char*s=0);
	gstr(gstr &);
	virtual ~gstr();
	inline uint32_t len() const { return _len; };
	operator const char*() const;
	gstr& operator=(gstr &);
	gstr& operator=(const char*);
	bool operator==(gstr &) const;
	bool operator==(const char*) const;
	char& at(int);
	char& operator [](int);
	int comp (gstr&); // -1, 0, 1
	int comp (const char *); // -1, 0, 1
	gstr& append(gstr&);
	gstr& append(const char*);
	gstr& operator += (gstr&);
	gstr& operator += (const char*);
	friend gstr operator + (gstr&,gstr&);
	friend gstr operator + (gstr&,const char*);
	gstr& operator <<= (gstr&);
	gstr& operator <<= (const char*);
	void lstrip(const char* d=0);
	void rstrip(const char *d=0);
	gstr& cut(int,int); // from to
	gstr& remove(const char*);
	gstr& remove_all(const char*);
	gstr& replace(const char*, const char* to=0);
	gstr& replace_all(const char*, const char* to=0);
	gstr& lower();
	gstr& upper();
};

#endif // __cplusplus

#endif // __GSTR_H__

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef __GSTR_H__
#define __GSTR_H__

typedef enum {
	eEND = 0,
	eALPA,     // constant, name
	eNUM,      // constant, 			
	eOP,       // +-*/?=!~^%&
	eDIL,      // ,.:;@$#
	eSPACE,    // isspace()
	eESC,      //
	eBLK,      // (, {, [, <, ", ', """, ''', /**/, #, 
	eERR = -1
} etoken_type;

typedef enum {
	eBLOCK=0,
	eLINE_COMMENT,
	eCOMMENT,
	eWIDE_CONSTANT,
	eCHAR_CONSTANT,
	eNOTBLOCK = -1
} eblock_type;

typedef enum {
	eARITHMETIC, //arithmetic  - +, -, *, /, %, ++, --
	eRELETIONAL, //relational  - ==, !=, >, <, >=, <=
	eLOGICAL,    //logical     - &&, ||, !
	eASSIGNMENT, //Assignment  - =, +=, -=, *=, /=, %=, <<=, >>=, &=, ^=, |=
	eBITWISE,    //Bitwise     - |, &, ^, ~, <<, >>
	eSPEICIAL,   //Special     - &,*,sizeof
	eCONDITION  //Conditional - ?:
//	ePOSTFIX     //            - ., ->, :
} eop_type;

typedef struct {
	eblock_type etype;
	const char *start;
	const char *end;
} block_type_t;

typedef struct {
	block_type_t *bt;
	int len;
} block_info_t;

#if 0
/*
Postfix	() [] -> . ++ - -	Left to right
Unary	+ - ! ~ ++ - - (type)* & sizeof	Right to left
*/
typedef struct {
   eop_type etype;
   const char*opstr;
} eop_type_t;
#endif

typedef struct {
	char *buf;
	size_t bsize;
	int len;
	int etype; // token type
	int subtype;  // sub type
	int step;
} ptoken_t;

#ifdef __cplusplus
extern "C" {
#endif

void touppers(char *str);
void tolowers(char *str);
int strchri(const char *s,int ch);
const char* whitespace_skip(const char *b);
const char *space_skip(const char *b,const char *s);
int cmpstr(const char *s,const char *d);
int instrs(const char *s,const char **dest);
int stoi(const char *s);
double stof(const char *s);
int64_t stol(const char *s);
int str2arr(char *msg, char **dest, size_t sz, const char *dil);
int str2arri(const char *msg, int16_t *dest, size_t sz, const char *dil);
char *strnzcpy(char *s1, const char *s2, size_t n);

int chop_str(char *msg, char **dest, size_t num, const char *dil);
int get_toktype(const char *);
const char *get_parse(const char* s, char *buf, size_t blen,int step);
const char *next_token(const char* s, ptoken_t *p);
const char *lookup_rblock(const char *str, block_type_t *et);
block_type_t *is_start_block(const char *str);
void parse(const char *s,int step);
int parse_block(block_type_t *bt, const char **str, char *buf, size_t bsize);
block_info_t get_block(const char **s, char *buf, size_t bsize, int step);

#ifdef __cplusplus
}

block_type_t *get_blktype(const char *s);
block_type_t *get_blktype(int idx);
const char *get_token(const char *s, char *buf, size_t blen);
const char *get_parse(const char *s, char *buf, size_t bsize, ptoken_t *ptoken=0);

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

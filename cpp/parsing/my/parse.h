#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef __PARSE__
#define __PARSE__

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

extern const char __default_white_space[];
extern const char __default_delimiter[];

void touppers(char *str);
void tolowers(char *str);
int strchri(const char *s,int ch);
const char* whitespace_skip(const char *b);
const char *space_skip(const char *b,const char *s);
const char *str_skip(const char *str,const char *skip);

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
const char *next_token(const char* s, ptoken_t *p);
const char *lookup_rblock(const char *str, block_type_t *et);
block_type_t *is_start_block(const char *str);

void parse(const char *s,int step);

int parse_block(block_type_t *bt, const char **str, char *buf, size_t bsize);
block_type_t *get_blktype(const char *s);
block_type_t *get_blktype_idx(int idx);
block_info_t get_block(const char **s, char *buf, size_t bsize, int step);
const char *get_block_info(const char *str, char *buf, size_t bsize, block_info_t *info);

const char *get_token(const char *s, char *buf, size_t blen);
const char *get_parse(const char* s, char *buf, size_t blen);
const char *get_parse_token(const char *s, char *buf, size_t bsize, ptoken_t *ptoken);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PARSE__

/*
 * gstr.cpp
 *
 *  Created on: Dec 14, 2021
 *      Author: seu
 */

#include <parse.h>
#include <ctype.h>
#include <malloc.h>

const char __default_white_space[] = " \t\n\r";
const char __default_delimiter[] = " \t\n\r,.:/";

void touppers(char *str) {
	while(*str) {
		*str = toupper(*str);
		str++;
	}
}

void tolowers(char *str) {
	while(*str) {
		*str = tolower(*str);
		str++;
	}
}

const char* whitespace_skip(const char *str) {
	if( !str ) 
		return (const char*)0;
	
	while( *str && strchr(__default_white_space,*str) ) str++;
	
	return str;
}

const char *str_skip(const char *str,const char *skip) {
	if( !str ) 
		return (const char*)0;
	
	if(skip && *skip)
		while(strchr(skip,*str)) str++;
	
	return str;
}

// return index
int strchri(const char *s,int ch) {
	if(!s || !ch) return -1;
	for(int i=0;*s; i++,s++) {
		if(*s == ch) return i;
	}
	return -1;
}

// strcmp  (*s2 - *s1)
// cmpstr  0: equal, 1: s > d, 'd' is fully included in 's', -1: error or not include
int cmpstr(const char *s,const char *d) {
	if( !s || !d ) return -1;

	for(;*s == *d;s++,d++) {
		if( !*d ) return 0;
	}
	//	return ((uint8_t)(*s) < (uint8_t)(*d))? -1 : 1;
	return (*d)? -1 : 1;
}

int instrs(const char *s,const char **dest) {
	if(!s || !dest) return -1;

	for(int i=0;*dest;i++,dest++) {
		if( cmpstr(s,*dest) == 0 )
			return i;
	}
	return -1;
}


#define isodigit(a) ((a) >= '0' && (a) <= '7')
#define isbdigit(a) ((a) == '0' || (a) == '1')

int stoi(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	int minus = 1;
	uint32_t val = 0;

	if(*s == '-' || *s == '+')  {
		if(*s++ == '-') minus = -1;
	}

	for(;*s >= '0' && *s <= '9'; s++) {
		val = (val * 10) + (*s - '0');
	}
	//if( val & 0x80000000 ) // error

	return (minus ==-1)? -(int)val : (int)val;
}

int64_t stol(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	int minus = 1;
	uint64_t val = 0;

	if(*s == '-' || *s == '+')  {
		if(*s++ == '-') minus = -1;
	}

	for(;*s >= '0' && *s <= '9'; s++) {
		val = (val * 10) + (*s - '0');
	}

	return (minus == -1)? -(int64_t)val : (uint64_t)val;
}

uint32_t stob(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	if(*s == '\0') {
		s++;
		if(*s == 'b' || *s == 'B') s++;
	}

	uint32_t val = 0;
	const char *n = s;

	while(isbdigit(*n)) n++;
	n--;

	for(uint32_t u=0; n>=s; n--, u <<= 1) {
		if( *n == '1' ) val += u;
	}

	return val;
}

uint32_t stoo(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	uint32_t val = 0;
	const char *n = s;

	while(isodigit(*n)) n++;
	n--;

	for(uint32_t u=0; n>=s; n--, u <<= 3) {
		val += (*n-'0') * u;
	}
	return val;
}

uint32_t stox(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	if(*s == '\0') {
		s++;
		if(*s == 'x' || *s == 'X') s++;
	}

	uint32_t val = 0;
	const char *n = s;

	while(isxdigit(*n)) n++;
	n--;

	for(uint32_t u=0; n>=s; n--, u <<= 4) {
		val += (*n-'0') * u;
	}
	return val;
}

double stof(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0.;

	int minus = 1;
	double v = 0;

	if(*s == '-' || *s == '+')  {
		if(*s++ == '-') minus = -1;
	}

	for(;*s && (*s >= '0' && *s <= '9'); s++) {
		v = (v * 10) + (*s - '0');
	}

	if(*s++ == '.') {
		int c;
		double d=1.0;
		for(c = 0; *s >= '0' && *s <= '9'; s++,c++) {
			d *= 0.1;
			v += d * (*s - '0');
		}
	}

	return (minus == -1)? -v : v;
}

// return field number, -1 when error or not effective field
int chop_str(char *msg, char **dest, size_t num, const char *dil) {
	int cnt=0;

	while(*msg && cnt < (int)num) {
		msg = (char *)str_skip((const char*)msg, __default_white_space);
		if( *msg == '\0' ) break;

		dest[cnt++] = msg;
		while(*msg && strchri(dil,*msg) == -1 ) msg++; // skip context
		if( *msg == '\0' ) break; // end

		*msg++ = '\0'; // must be *msg is dil or white space
	}

	return cnt;
}

// s1 > (n+1)
char *strnzcpy(char *s1, const char *s2, size_t n)
{
	char *s = s1;
	while (n > 0 && *s2 != '\0') {
		*s++ = *s2++;
		--n;
	}
	while (n > 0) {
		*s++ = '\0';
		--n;
	}
	*s = '\0';
	return s1;
}

const char *get_token(const char *s, char *buf, size_t bsize) {
	size_t cp = 0;

	buf[0] = '\0';
	s = whitespace_skip(s);
	if( !s || !*s )
		return (const char *)0;

	while(*s) {
		if(isspace(*s)) {
			break;
		}
		buf[cp++] = *s++;
		if(bsize < (cp+2)) break;
	}
	buf[cp] = '\0';
   s = whitespace_skip(s);

	return (cp > 0)? s : 0;
}

static const char __block_chars[] = "'\"(){}[]<>";
static const char __block_start_chars[] = "/'\"({[<";
static const char __block_end_chars[] = "'\")}]>*";

bool is_block_string(const char *str) {
	return (strchr(__block_chars, *str))? 1 : 0;
}

#if 0
typedef enum {
	e_block,
	e_op,
	e_num,
	e_name,
	e_comment,
	e_sepe,
	e_space,
	e_end = -1,
	e_err = -2
} parse_type_t;

typedef struct {
	parse_type_t	type;
	const char *str;
} fam_strs_t;

const fam_strs_t __fam_strs[] = {
	{ e_block, "\"\"\"" }, // python
	{ e_block, "\'\'\'" }, // python
	{ e_comment, "/*" },
	{ e_comment, "*/" },
	{ e_comment, "//" },
	{ e_op, "<<=" },
	{ e_op, ">>=" },
	{ e_op, ">>" },
	{ e_op, "<<" },
	{ e_op, "==" },
	{ e_op, "<=" },
	{ e_op, ">=" },
	{ e_op, "!=" },
	{ e_op, "&&" },
	{ e_op, "||" },
	{ e_op, "&=" },
	{ e_op, "|=" },
	{ e_op, "^=" },
	{ e_op, "!=" },
	{ e_op, "*=" },
	{ e_op, "/=" },
	{ e_op, "+=" },
	{ e_op, "-=" },
	{ e_op, "->" },
	{ e_op, "%=" },
	{ e_op, "~=" },
	{ e_op, "++" },
	{ e_op, "--" },
	{ e_block, "\"" },
	{ e_block, "\'" },
	{ e_block, "\n" },
//	{ e_comment, "#" },
	{ e_op, "!" },
	{ e_op, "~" },
	{ e_op, "!" },
	{ e_op, "=" },
	{ e_op, "+" },
	{ e_op, "-" },
	{ e_op, "*" },
	{ e_op, "/" },
	{ e_op, "%" },
	{ e_op, "^" },
	{ e_op, "|" },
	{ e_op, "&" },
	{ e_block, "(" },
	{ e_block, ")" },
	{ e_block, "[" },
	{ e_block, "]" },
	{ e_block, "{" },
	{ e_block, "}" },
	{ e_block, "<" },
	{ e_block, ">" },
	{ e_sepe, "," },
	{ e_op, "." },
	{ e_sepe, "@" },
	{ e_sepe, "$" },
	{ e_sepe, ";" },
	{ e_sepe, "#" },
	{ e_op, "?" },
	{ e_op, ":" },
	{ e_end, 0 }
};
#endif

int get_toktype(const char *s) {
	if(!s) return eERR;
	if(!*s) return eEND;
	if(isalpha(*s) || *s == '_') return eALPA;
	if(isspace(*s)) return eSPACE;
	if(isdigit(*s)) return eNUM;
	if(*s == '\\') {
		return eESC;
	}
	if(strchr("+-*/%|&^=<>!~",*s)) {
		switch(*s) {
		case '/':
			if( s[1] == '*' ) return eBLK;
			if( s[1] == '/' ) return eBLK;
			break;
		case '*':
			if( s[1] == '/' ) return eBLK;
			break;
		case '<': // <, <<, <<=, <=,
			if( s[1] != '=' && s[1] != '<' ) return eBLK;
			break;
		case '>': // >, >>, >>=, >=
			if( s[1] != '=' && s[1] != '>' ) return eBLK;
			// ref. prev and next token 
			break;
		case '-':
			if( s[1] != '>' ) return eDIL;
			break;
		default:;
		}
		return eOP;
	}
	if(is_block_string(s)) return eBLK;
	if(strchr("`:;.,?@#$_",*s)) return eDIL;
	return eERR;
}

block_type_t __block_types[] = {
	{ eWIDE_CONSTANT, "\"\"\"", "\"\"\"" },
	{ eWIDE_CONSTANT, "'''", "'''" },
	{ eCOMMENT, "/*", "*/" },
//	{ eCOMMENT2, "\"\"\"", "\"\"\"" },
	{ eLINE_COMMENT, "//", "\n" },
	{ eLINE_COMMENT, "#", "\n" },
	{ eLINE_COMMENT, "--", "\n" },
	{ eCHAR_CONSTANT, "\'", "\'" },
	{ eCHAR_CONSTANT, "\"", "\"" },
	{ eBLOCK, "(", ")" },
	{ eBLOCK, "{", "}" },
	{ eBLOCK, "[", "]" },
	{ eBLOCK, "<", ">" },
	{ eNOTBLOCK,0,0 }

};

block_type_t *is_start_block(const char *str) {
	block_type_t *b = __block_types;

	if( !str || !*str ) return (block_type_t*)0;

	for( int i=0;b->etype != eNOTBLOCK; i++,b++) {
		if(cmpstr(str,b->start) >= 0) {
			return b;
		}
	}

	return (block_type_t*)0;
}

block_type_t *is_end_block(const char *str) {
	block_type_t *b = __block_types;

	if( !str || !*str ) return (block_type_t*)0;

	for( int i=0;b->etype != eNOTBLOCK; i++,b++) {
		if(cmpstr(str,b->end) >= 0) {
			return b;
		}
	}

	return (block_type_t*)0;
}

block_type_t *get_blktype_idx(int e) {
	int i=0;
	while( __block_types[i].etype != eNOTBLOCK ) {
		if( __block_types[i].etype == e ) 
			return &__block_types[i];
		i++;
	}
	return (block_type_t*)0;
}

block_type_t *get_blktype(const char *s) {
	if( get_toktype(s) != eBLK ) return (block_type_t *)0;
	
	block_type_t *e = is_start_block(s);
	if(e) return e;

	return is_end_block(s);
}

// return nextp, fill buffer except block string, 
const char *get_block_info(const char *str, char *buf, size_t bsize, block_info_t *info) {
	if(!str || !buf || bsize < 2)
		return str;

	buf[0] = '\0';

	const char *s = lookup_rblock(str, info->bt);

	info->len = (s - str);
	if( info->len < bsize ) {
		strncpy(buf, str, info->len);
		buf[info->len] = '\0';
	}

	return s;
}

block_info_t get_block(const char **str, char *buf, size_t bsize, int step) {
	block_info_t bi = { 0,0 };

	if(!str || !buf || bsize < 2)
		return bi;

	if((bi.bt = is_start_block(*str)) == 0) 
		return bi;

	*str += strlen(bi.bt->start);

	buf[0] = '\0';
	const char *bs,*n;
	bs = *str;
// TEST
printf("begin step=%d bt(%d|%s)\n",step,bi.bt->etype,bi.bt->start);

	if(bi.bt->etype == eCHAR_CONSTANT) {
		const char *s = strchr(*str, '\n');
		n = (const char*)strstr(*str,bi.bt->end);
		if( s < n && s ) { n = s; }
		if( !n ) {
			n = *str + strlen(*str);
		} 
		*str = n + 1;
		bi.len = (n-bs);
		strncpy(buf,bs,bi.len);
		buf[bi.len] = '\0';

		return bi;
	} 

	if(bi.bt->etype == eLINE_COMMENT) {
		const char *s = strchr(*str, '\n');
		if( !s ) 
			bi.len = strlen(bs);
		else { 
			*str = s + strlen(bi.bt->end);
			bi.len = (s - bs);
		}
		strncpy(buf,bs,bi.len);
		buf[bi.len] = '\0';
		
		return bi;
	}

	if(bi.bt->etype == eWIDE_CONSTANT || bi.bt->etype == eCOMMENT) {
		const char *s = strstr(*str, bi.bt->end);
		if( !s ) 
			bi.len = strlen(bs);
		else {
			bi.len = (s - bs);
			*str = s + strlen(bi.bt->end);
		}
		strncpy(buf,bs,bi.len);
		buf[bi.len] = '\0';
		
		return bi;
	}

	block_info_t bis={0,0},bie={0,0};
// TEST
 printf("start step=%d bt(%s) [%s]\n",step,bi.bt->start,*str);
	do {
#if 1		
		while(**str) {
			block_type_t *e = get_blktype(*str);
			if( !e ) {
//				printf("not block (%c)\n",**str);
			} else {
//				printf("ok block (%c-%d)\n",**str,e->etype);
			}
			if( e && e->etype == eBLOCK) break; 
			(*str)++; 
			}
#else
		while(**str) {
			if( get_toktype(*str) != eBLK ) break;
			(*str)++; 
			}
#endif
		if(!**str) { break; }

//printf("++++[%s]\n",*str);
		bis.bt=is_start_block(*str);
		if(bis.bt) {
			bis = get_block(str, buf, bsize, step+1);
			if( bis.len >= 0 ) {
			} else {
				// error
			}
			continue;
		} 
		bie.bt=is_end_block(*str);
		if(bie.bt) {
//printf("^^^^[%p/%p (%s/%s)] (%s)",bi.bt, bie.bt, bi.bt->end, bie.bt->end,*str);				
			if( bi.bt == bie.bt ) {
				bi.len = (*str - bs); 
				*str += strlen(bie.bt->end);
//printf("^^[ (%d/%d - <%s|%s>)]",bi.len,bsize, *str,bs);				
				if( (size_t)bi.len >= bsize ) { 
					bi.len = -bi.len;
					return bi; 
				}
				strncpy(buf, bs, bi.len);
				buf[bi.len] = '\0';

//				printf("[B%d<%s>]\n",step+1,buf);
				parse(buf, step+1);

				return bi; // return valid
			} else {
//printf("^^^^BAD conext ^^^^^^^^[%p/%p (%s/%s)] (%s)",bi.bt, bie.bt, bi.bt->end, bie.bt->end,*str);				
				// \n /* */
			}
		} else {
printf("::::::::::::inner error:::::::::");				
		}

		(*str)++;
	} while(**str);
	
	bi.len = (*str - bs);
	strncpy(buf, bs, bi.len);
	buf[bi.len] = '\0';

	return bi;
}

const char *get_parse(const char *s, char *buf, size_t bsize) {
  return get_parse_token(s,buf,bsize,(ptoken_t *)0);
}

// return next strp, throw away useless garbage.
const char *get_parse_token(const char *s, char *buf, size_t bsize, ptoken_t *ptoken) {
	if( !s || !buf || bsize < 2) return 0;

	ptoken_t pt, *p;
	p = (ptoken)? ptoken : &pt;
	
	size_t blen = bsize;
	int nt=eEND;
	p->etype = 0; 
	p->subtype = 0;
	p->len = 0;

#define CHK_LEN do {\
		buf[p->len++] = *s++;\
		if(bsize < (size_t)(p->len+1)) { buf[p->len] = 0; return s; }\
} while(0)

	s = whitespace_skip(s);
	if( !s ) {
		return 0;
	}

	p->etype = get_toktype(s);
	if(p->etype == eEND || p->etype == eERR) return 0;
	buf[p->len++] = *s++; buf[p->len] = '\0'; blen--;

//printf("(%d)get_parse p->etype=%d(%c)\n",step,p->etype,buf[0]);	
	if(p->etype == eOP) {
		if(buf[0] == '+' || buf[0] == '-') { // effective decimal only, not hexa, bin, octal
			if(get_toktype(s) == eNUM) { // +-number
				p->etype = eNUM;
				return s;
				//return get_parse(s, buf, bsize, p);
			} else { // ++,+=, --, -=, ->
				if((buf[0] == '+' && (*s == '+' || *s == '=')) ||
						(buf[0] == '-' && (*s == '-' || *s == '=' || *s == '>'))) {
					CHK_LEN;
				}
				buf[p->len] = '\0';
				return s;
			}
		} else
		if(buf[0] == '<') { // <<, <=, <<=
			if( *s == '<' ) {
				CHK_LEN;
				if( *s == '=' ) {
					CHK_LEN;
				}
			} else
			if( *s == '=' ) {
					CHK_LEN;
			}
		} else
		if(buf[0] == '>') { // >>, >=, >>=
			if( *s == '>' ) {
				CHK_LEN;
				if( *s == '=' ) { // >>=
					CHK_LEN;
				}
			} else
			if( *s == '=' ) { // >=
				CHK_LEN;
			}
		} else
		if(buf[0] == '=') { // ==
			if( *s == '=' ) {
				CHK_LEN;
			}
		} else
		if(buf[0] == '!') { // !=
			if( *s == '=' ) {
				CHK_LEN;
			}
		} else
		if(buf[0] == '|') { // ||, |=
			if( *s == '=' || *s == '|' ) {
				CHK_LEN;
			}
		} else
		if(buf[0] == '&') { // &&, &=
			if( *s == '=' || *s == '&' ) {
				CHK_LEN;
			}
		} else
		if(buf[0] == '/') { // /*
			if( *s == '*' ) { // 
				CHK_LEN;
			}
		} else
		if(buf[0] == '*') { // */
			if( *s == '/' ) { // 
				CHK_LEN;
			}
		} else
		if(buf[0] == '~') { //
			if( *s == '=' ) { // ~=
				CHK_LEN;
			}
		} else
		if(buf[0] == '^') { // ^^,^=
			if( *s == '=' || *s == '^' ) {
				CHK_LEN;
			}
		}
		buf[p->len] = 0;
		return s;
	}

	if(p->etype == eESC) {
		if(*s == 'n' || *s == 'r' || *s == 't') {
			buf[p->len-1] = (*s=='n')? '\n' : (*s=='r')? '\r' : '\t';
		} else {
			buf[p->len-1] = *s;
		}
		buf[p->len] = '\0';
		return (s+1);
	}

	if(*(s-1) == '0') { // check next char, 0x, 0b, 0
		if(*s == 'b') { // bin 0b1023424
			CHK_LEN;
			while(*s == '0' || *s == '1') {
				CHK_LEN;
			}
			buf[p->len] = '\0';
			return s;
		}
		if(*s == 'x') { // hexa 0xf23a
			CHK_LEN;
			while(isxdigit(*s)) {
				CHK_LEN;
			}
			buf[p->len] = '\0';
			return s;
		}
	}

	if(p->etype == eNUM) {
		while(isdigit(*s)) {
			CHK_LEN;
		}
		if(*s == '.') {
			CHK_LEN;
			while(isdigit(*s)) {
				CHK_LEN;
			}
		}
		buf[p->len] = '\0';
		return s;
	}

	if(p->etype == eBLK) {
		block_type_t *bt = get_blktype(s-1);
		p->subtype = bt->etype;

		const char *tmps = lookup_rblock(s, bt);
		if( !tmps ) {
			return 0;  // context invalid
		}
		p->len = tmps - s;
		if(p->len != 0) { 
			strncpy(buf, s, p->len);
		}
		buf[p->len] = '\0';

printf("buf=[%s], next=[%s][%s]\n", buf, tmps,bt->end); 
		return (tmps);
	}

	if(p->etype == eDIL) {
		buf[p->len] = '\0';
		return s;
	}

	while((nt=get_toktype(s)) == p->etype) {
		CHK_LEN;
	}

	// C name
	if( nt == eNUM && p->etype == eALPA ) {
		while(isdigit(*s) || get_toktype(s) == eALPA) {
			CHK_LEN;
		}
	}
	buf[p->len] = '\0';

	return whitespace_skip(s);
}
#undef CHK_LEN

const char *next_token(const char *s, ptoken_t *p) {
	if( !s || !*s || !p || p->bsize < 2) return 0;

	size_t blen = p->bsize;
	int nt=eEND;
	p->buf[0] = '\0';
	p->etype = 0; 
	p->subtype = 0;
	p->len = 0;

#define CHK_LEN do {\
		p->buf[p->len++] = *s++;\
		if(p->bsize < (size_t)(p->len+1)) { p->buf[p->len] = 0; return s; }\
} while(0)

	s = whitespace_skip(s);
	if( !s ) {
		return 0;
	}

	p->etype = get_toktype(s);
	if(p->etype == eEND || p->etype == eERR) return 0;
	p->buf[p->len++] = *s++; p->buf[p->len] = '\0'; blen--;
// TEST
printf("get_parse p->etype=%d(%c)\n",p->etype,p->buf[0]);	
	if(p->etype == eOP) {
		if(p->buf[0] == '+' || p->buf[0] == '-') { // effective decimal only, not hexa, bin, octal
			if(get_toktype(s) == eNUM) { // +-number
				p->etype = eNUM;
				return next_token(s, p);
			} else { // ++,+=, --, -=, ->
				if((p->buf[0] == '+' && (*s == '+' || *s == '=')) ||
						(p->buf[0] == '-' && (*s == '-' || *s == '=' || *s == '>'))) {
					CHK_LEN;
				}
				p->buf[p->len] = '\0';
				return s;
			}
		} else
		if(p->buf[0] == '<') { // <<, <=, <<=
			if( *s == '<' ) {
				CHK_LEN;
				if( *s == '=' ) {
					CHK_LEN;
				}
			} else
			if( *s == '=' ) {
					CHK_LEN;
			}
		} else
		if(p->buf[0] == '>') { // >>, >=, >>=
			if( *s == '>' ) {
				CHK_LEN;
				if( *s == '=' ) { // >>=
					CHK_LEN;
				}
			} else
			if( *s == '=' ) { // >=
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '=') { // ==
			if( *s == '=' ) {
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '!') { // !=
			if( *s == '=' ) {
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '|') { // ||, |=
			if( *s == '=' || *s == '|' ) {
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '&') { // &&, &=
			if( *s == '=' || *s == '&' ) {
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '/') { // /*
			if( *s == '*' ) { // 
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '*') { // */
			if( *s == '/' ) { // 
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '~') { //
			if( *s == '=' ) { // ~=
				CHK_LEN;
			}
		} else
		if(p->buf[0] == '^') { // ^^,^=
			if( *s == '=' || *s == '^' ) {
				CHK_LEN;
			}
		}
		p->buf[p->len] = 0;
		return s;
	}

	if(p->etype == eESC) {
		if(*s == 'n' || *s == 'r' || *s == 't') {
			p->buf[p->len-1] = (*s=='n')? '\n' : (*s=='r')? '\r' : '\t';
		} else {
			p->buf[p->len-1] = *s;
		}
		p->buf[p->len] = '\0';
		return (s+1);
	}

	if(*(s-1) == '0') { // check next char, 0x, 0b, 0
		if(*s == 'b') { // bin 0b1023424
			CHK_LEN;
			while(*s == '0' || *s == '1') {
				CHK_LEN;
			}
			p->buf[p->len] = '\0';
			return s;
		}
		if(*s == 'x') { // hexa 0xf23a
			CHK_LEN;
			while(isxdigit(*s)) {
				CHK_LEN;
			}
			p->buf[p->len] = '\0';
			return s;
		}
	}

	if(p->etype == eNUM) {
		while(isdigit(*s)) {
			CHK_LEN;
		}
		if(*s == '.') {
			CHK_LEN;
			while(isdigit(*s)) {
				CHK_LEN;
			}
		}
		p->buf[p->len] = '\0';
		return s;
	}

	if(p->etype == eBLK) {
		block_type_t *bt = get_blktype(s-1);
		p->subtype = bt->etype;

		const char *tmps = lookup_rblock(s, bt);
		if( !tmps ) {
			return 0;  // context invalid
		}
		p->len = tmps - s;
		if(p->len != 0) { 
			strncpy(p->buf, s, p->len);
		}
		p->buf[p->len] = '\0';

		return (s + p->len + strlen(bt->end));
	}

	if(p->etype == eDIL) {
		p->buf[p->len] = '\0';
		return s;
	}

	while((nt=get_toktype(s)) == p->etype) {
		CHK_LEN;
	}

	// C name
	if( nt == eNUM && p->etype == eALPA ) {
		while(isdigit(*s) || get_toktype(s) == eALPA) {
			CHK_LEN;
		}
	}
	p->buf[p->len] = '\0';

	return s; //whitespace_skip(s);
}
#undef CHK_LEN

const char *lookup_rblock(const char *str, block_type_t *et) {
	//block_type_t *is_end_block(const char *str)
	// skip space
	// get_toktype -- ! BLK skip
	// start_block() --> lookup_rblock()
	// end_block() --> return

	if( !str || !et ) return 0;

	int ty;
	block_type_t *t;
	
	while(str && *str) {
		str = whitespace_skip(str);
		if(!str) break;
		ty = get_toktype(str);
		if( ty == eBLK ) {
			if(is_end_block(str) == et) {
//printf("et->end = [%s] str = [%s]\n", et->end, str+strlen(et->end));			
				return str+strlen(et->end);
			}
			
			t = is_start_block(str);
			if( t ) {
				str = lookup_rblock(str+strlen(t->start), t);
			} else {
			}
		} else {
			str++;
		}
	}
	return str;
}

ptoken_t *get_ptoken(const char *str, ptoken_t *pt) {
	pt->bsize = strlen(str)+1;
	pt->buf = (char*)malloc(pt->bsize);
	if( !pt->buf ) return 0;
	pt->etype = -1;
	pt->subtype = -1;
	pt->len = 0;

	return pt;
}

void parse(const char *s,int step) {
	char *buf = (char*)malloc(1024);

	while(s && *s) {
		s = get_parse(s, buf, 1023);
		if( !s ) break;

		printf("%ld|[%s]\n",strlen(buf),buf);
	}
}

int _parse(const char *s,int step) {
	if(!s) return -1;
	uint32_t cnt=0;

//	block_info_t bi;
	ptoken_t pt;
	get_ptoken(s, &pt);
	pt.step = step;

//printf("(%d)start parse len=%u\n",step,len+1);
	while(*s) {
		s = next_token(s, &pt);
		if(!s) break;
		printf("[%d|%d|%s(%d)]\n",step,pt.etype,pt.buf,pt.len);
#if 0		
		if(pt.etype == eBLK && pt.subtype == eBLOCK) {
			bi = get_block(&s,pt.buf, pt.bsize, pt.step+1);
			pt.subtype = bi.bt->etype;
			pt.len = bi.len;
//			parse(s, pt.step+1);
		}
#endif			
		cnt++;
	}

	printf("(%d)total token = %u blen=%lu\n",pt.step,cnt,pt.bsize);
	if(pt.bsize > 0) { 
		free(pt.buf); //delete[](pt.buf);
    pt.buf = 0;
  }

	return (int)cnt;
}

/////////////////////////////////////////////////////////////////////////////////
#ifdef TEST
#include <math.h>

#define isfloat(v) (((v)-((int64_t)(v))) != 0.)

void calc(const char *s) {
	char buf[128];
	strcpy(buf,s);
//	s = get_token(s, buf);

//	auto help [](int i) { printf("$ calc xx op yy [%d]\n",i); };

	double v1,v2, rv;
	int64_t u,ru;
	int op;
	if((s = get_parse(s, buf, 128))) {
		v1 = stof(buf);
	}
		printf("--%s\n",buf);
	if((s = get_parse(s, buf, 128))) {
		op = buf[0];
		printf("--%s\n",buf);
		if(op == '!') {
			goto aa;
		}
		if(op == '+' || op == '-') {
			if( isdigit(buf[1]) ) {
				v2 = stof(buf+1);
				goto aa;
			}
		}
	}
	if((s = get_parse(s, buf, 128))) {
		v2 = stof(buf);
	}
aa:
	switch( op ) {
	case '+':
		rv = v1 + v2;
		break;
	case '-':
		rv = v1 - v2;
		break;
	case '*':
		rv = v1 * v2;
		break;
	case '/':
		rv = v1 / v2;
		break;
	case '^':
		rv = pow(v1,(uint32_t)v2);
		printf(">>%lf\n",rv);
		return;
	case '%':
		printf(">>%u\n",(uint32_t)v1 % (uint32_t)v2);
		return;
	case '!':
		u = (int64_t)v1;
		ru = 1;
		for(int i=0; u>0; i++, u--) {
			ru *= u;
		}
		printf(">>%u factorial is (%lu)\n",(int32_t)v1,ru);
		return;
	default:
//		help(-1);
		return;
	}
	if(!isfloat(rv)) {
		printf(">>%d\n",(int32_t)rv);
	} else {
		printf(">>%lf\n",rv);
	}
}

int factorial(int num) {
	uint32_t v=num;
	for(int i=num-1; i>0 ; i--) {	v += i; }
	return v;
}

double calc2(const char *str) {
	ptoken_t pt;
	char buf[128];
	char ops[8];

	double vr=0., v=0.;
	str = get_parse_token( str, buf, sizeof(buf)-1, &pt);
	if(!str) return v;
		
printf("v1 = %f [%s]\n",v,str);
	if(pt.etype == eBLK && pt.subtype == eBLOCK) {
		v = calc2(buf);
	} else if(pt.etype == eNUM) {
		v= stof(buf);
	} else return v;

printf("v2 = %f [%s]\n",v,str);
	while(*str) {
		str = get_parse_token( str, buf, sizeof(buf)-1, &pt);
		if(!str) return v;
		if(pt.etype != eOP) return v;
		
		if(buf[0] == '!')
			return factorial(v);

		strcpy(ops,buf);
printf("v3 = %f [%s]\n",v,str);
		str = get_parse_token( str, buf, sizeof(buf)-1, &pt);
		if(!str) break;
		
		if(pt.etype == eBLK && pt.subtype == eBLOCK) {
			vr = calc2(buf);
		} else if(pt.etype == eNUM) {
			vr = stof(buf);
		}

printf("v4 = %f, vr = %f [%c]\n",v,vr,ops[0]);

			switch(ops[0]) {
			case '+':
				if( buf[1] == '+' ) return v;

				v += vr;
				break;
			case '-':
				if( buf[1] == '-' ) return v;
				
				v -= vr;
				break;
			case '*':
				v *= vr;
				break;
			case '/':
				v /= vr;
				break;
			case '^':
				v = pow(v,vr);
				break;
			case '%':
				v = (double)((uint32_t)v % (uint32_t)vr);
				break;
#if 0			
			case '&':
				;
				break;
			case '|':
				op = 10;
				break;
#endif				
			default:;
				break;
			}
	}

	return v;
}

void calc2t(const char *str) {
	double v = calc2(str);

	if( isfloat(v) ) {
		printf("%s = %f\n", str, v);
	} else {
		printf("%s = %ld\n", str, (long)v);
	}
}

int ld_file(const char *filename, char**buf)
{
	int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL)
	{
		*buf = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*buf = (char *)malloc(size+1);
	if(!*buf) {
		fclose(f);
		return -3;
	}
	if (size != fread(*buf, sizeof(char), size, f))
	{
		free(*buf);
		*buf = 0;
		return -2; // -2 means file reading fail
	}
	fclose(f);
	return size;
}

int main(int argc,char **argv) {
	if(argc < 1) return 1;

//	calc2t(argv[1]); return 0;

	char *bf=0;
	int sz = ld_file(argv[1], &bf);
	printf("[%s] fsize=%d\n", argv[1],sz);
	if(sz < 0 || !bf) {
		return 1;
	}
	parse(bf,0);

	free(bf);
}

#endif // TEST

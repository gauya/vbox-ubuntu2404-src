#include <gstr.h>

#ifdef __cplusplus

#include <stdexcept>
using namespace std;

gstr::gstr(const char *s) {
	// TODO Auto-generated constructor stub
	if(s) {
		_len = strlen(s);
		_str = new char[_len+1];
		if( !_str ) {
			// throw
		}
		strcpy(_str,s);
		_bsize = _len+1;
	} else {
		_str = 0;
		_len = 0;
		_bsize = 0;
	}
}

gstr::gstr(gstr &g) {
	// TODO Auto-generated constructor stub
	if(g.len() > 0) {
		_len = g.len();
		_str = new char[_len+1];
		if( !_str ) {
			// throw
		}
		strcpy(_str,g._str);
		_bsize = _len+1;
	} else {
		_str = 0;
		_len = 0;
		_bsize = 0;
	}
}

gstr::~gstr() {
	// TODO Auto-generated destructor stub
	if(_bsize > 0) {
		delete[] (_str);
	}
	_str = 0;
	_len = _bsize = 0;
}

gstr::operator const char*() const{
	return (const char*)_str;
}

gstr& gstr::operator=(gstr &g) {
	if( _bsize > 0 ) {
		delete[] _str;
	}
	if( g.len() > 0 ) {
		_str = new char[g.len()+1];
		_len = g.len();
		_bsize = g._len+1;
	} else {
		_str = 0;
		_len = 0;
		_bsize = 0;
	}
	return *this;
}
#include <string.h>

gstr& gstr::operator=(const char*s) {
	if( _bsize > 0 ) {
		delete[] _str;
		_bsize = _len = 0;
		_str = 0;
	}
	if( s ) {
		_len = strlen(s);
		_str = new char[_len+1];
		strcpy(_str,s);
		_bsize = _len + 1;
	} else {
		_str = 0;
		_len = 0;
		_bsize = 0;
	}
	return *this;
}
bool gstr::operator==(gstr &g) const {
	return (g.len() == len() && strcmp(g._str, _str) == 0)? true : false;
}

bool gstr::operator==(const char*s) const {
	return (strlen(s) == _len && strcmp(s, _str) == 0)? true : false;
}

char& gstr::at(int idx) {
	if( (uint32_t)idx < _len ) {
		return _str[idx];
	} else {
		//return _str[0];
		throw invalid_argument("MyFunc argument too large.");
		//		throw std::out_of_range("");
		//		throw std:: length_error(idx);
	}
}

char& gstr::operator [](int idx) {
	if( (uint32_t)idx < _len ) {
		return _str[idx];
	} else {
		throw std::out_of_range("");
	}
}

int gstr::comp (gstr& g) { // -1, 0, 1
	return strcmp(_str, g._str);
}

int gstr::comp (const char *s) { // -1, 0, 1
	return strcmp(_str,s);
}

gstr& gstr::append(gstr& g) {
	return append((const char*)g);
}

gstr& gstr::append(const char*s) {
	char *ns;
	uint32_t ln = strlen(s);
	if( ln == 0 ) return *this;

	int fb = _bsize - _len -1;
	if( fb < (int)ln ) {
		ns = new char[ln + len() + 1];
		strcpy(ns, _str);
		strcpy(ns+len(), s);

		if( _bsize > 0 )
			delete[] _str;
		_bsize = ln + len() + 1;
		_str = ns;
		_len += ln;
	} else {
		strcpy(_str + _len, s);
		_len += ln;
	}

	return *this;
}

gstr& gstr::operator += (gstr& g) {
	return append(g);

}

gstr& gstr::operator += (const char*s) {
	return append(s);
}

gstr& gstr::operator <<= (gstr& g) {
	return append(g);
}

gstr& gstr::operator <<= (const char* s) {
	return append(s);
}

gstr operator + (gstr& g1, gstr& g2) {
	gstr a = g1;
	a.append(g2);
	return a;
}

gstr operator + (gstr& g, const char* s) {
	gstr a(g);
	a.append(s);
	return a;
}

void gstr::lstrip(const char *d) {
	const char *s;
	if(!d) {
		s = whitespace_skip(_str);
	} else {
		s = str_skip(_str, d);
	}

	if( s != _str ) {
		// memcpy(
		uint32_t sz = (s - _str);
		for(int i=0; sz < _len; i++) {
			_str[i] = _str[sz++];
		}
		_str[sz] = '\0';
		_len -= (sz - 1);
		_bsize = 0;
	}
}

void gstr::rstrip(const char *d) {
	if( len() < 1 ) return;
	const char *p = (!d)? __default_white_space : d;
	for(; _len >= 0; _len--) {
		if( strchri(p,_str[_len]) == -1 ) {
			return;
		}
		_str[_len] = '\0';
		if(_bsize>=_len) _bsize = _len - 1;
	}
}

// strstr, strcasestr
gstr& gstr::remove(const char*ss) {
	if(!ss) return *this;
	char *s = strstr(_str,ss);
	if(!s) return *this;

	uint32_t ll=strlen(ss);

	strcpy(s, s+ll);
	_len -= ll;

	return *this;
}

gstr& gstr::remove_all(const char*ss) {
	if(!ss) return *this;
	char *s = strstr(_str,ss);
	if(!s) return *this;

	uint32_t ll=strlen(ss);

	do {
		strcpy(s,s+ll);
		s = strstr(s,ss);
	} while(s);

	_len = strlen(_str);

	return *this;
}

gstr& gstr::replace(const char*s1, const char*s2) {
	if(!s1) return *this;
	char *s = strstr(_str,s1);
	if(!s) return *this;

	if(!s2) return remove(s1);

	uint32_t l1=strlen(s1),l2=strlen(s2);

	auto scp = [](char*s,const char*s2){ while(*s2){ *s++ = *s2++; }; return s; };

	if( l1 == l2 ) {
		scp(s,s2);
		return *this;
	}

	if( l1 > l2 ) {
		const char *n = s + l1;
		s = scp(s,s2);
		s = scp(s,n);
		*s = '\0';

		_len -= (l1-l2);
		return *this;
	}

	char *nstr = new char[_len + l2 + 1];
	if(!nstr) return *this;

	char *np = nstr;
	strnzcpy(np,_str,(s - _str)); np += (s - _str);
	scp(np,s2); np += l2;
	strcpy(np,s+l1);

	if( _bsize > 0 ) delete _str;

	_bsize = _len + l2 + 1;
	_len = _len + (l2-l1);
	_str = nstr;

	return *this;
}

gstr& gstr::replace_all(const char*s1, const char*s2) {
	if(!s1) return *this;
	char *s = strstr(_str,s1);
	if(!s) return *this;

	if(!s2) return remove_all(s1);

	uint32_t l1=strlen(s1),l2=strlen(s2);

	auto scp = [](char*s,const char*s2){ while(*s2){ *s++ = *s2++; }; return s; };
	// l1 == l2
	if( l1 == l2 ) {
		scp(s,s2);
		return *this;
	}

	char *p1, *p2;
	// l1 > l2
	if( l1 > l2 ) {
		do {
			p2 = s + l2;
			p1 = s + l1;
			scp(s,s2);
			s = strstr(p1,s1);

		} while(s);
		strcpy(p2,p1);

		_len = strlen(_str);

		return *this;
	}

	char *nstr = new char[_len + l2 + 1];
	p2 = nstr;
	p1 = _str;
	int ln;
	do {
		ln = s-p1;
		strncpy(p2, p1, ln);
		s += l1;
		p1 = s;
		p2 += ln;
		strncpy(p2, s2, l2);
		p2 += l2;

		s = strstr(p1, s1);
	} while(s);
	if(!s) {
		strcpy(p2, p1);
	}

	if( _bsize > 0 ) delete _str;
	_bsize = _len + l2 + 1;
	_len = strlen(nstr);
	_str = nstr;

	return *this;
}

gstr& gstr::cut(int from, int to) {
	if( to < 0 ) to = _len - 1;
	if(from >= to || from < 0 || (uint32_t)to >= _len)
		return *this;

	strcpy(_str+from, _str+to+1);

	_len -= (to-from+1);

	return *this;
}

gstr& gstr::lower() {
	if( _len > 0 ) {
		for(uint32_t i=0;i<_len;i++) {
			_str[i] = tolower(_str[i]);
		}
	}
	return *this;
}

gstr& gstr::upper() {
	if( _len > 0 ) {
		for(uint32_t i=0;i<_len;i++) {
			_str[i] = toupper(_str[i]);
		}
	}
	return *this;
}

#endif // __cplusplus

/**
 * @brief 无分配字符串封装类
 * @details 无分配字符串封装类，有效扩展 char 字符串操作
 * @author tomken
 * @data 2016-06-01
 * 
 */

#ifndef __SCOPED_STRING_H__
#define __SCOPED_STRING_H__

#include <string.h>

#define IS_WHITESPACE(x) (' ' == (x) || '\t' == (x) || '\r' == (x) || '\n' == (x))

class ScopedString {
public:
    static const unsigned stringHashingStartValue = 0x9E3779B9U;
    
    ScopedString() {
        _data = 0;
        _size = 0;
        _hash = 0;
    }
    
    ScopedString(const char* str) {
        if (str) {
            _data = str;
            _size = (unsigned)strlen(str);
        } else {
            _data = 0;
            _size = 0;
        }
        
        _hash = 0;
    }
    
    ScopedString(const char* data, unsigned size) {
        _data = data;
        _size = size;
        _hash = 0;
    }
    
    ScopedString(const ScopedString& str) {
        _data = str._data;
        _size = str._size;
        _hash = str._hash;
    }
    
public:
    bool isNull() const {
        return (_size == 0 || _data == 0);
    }
    unsigned size() const {
        return _size;
    }
    
    const char* data() const {
        return _data;
    }
    
    const char* tail() const {
        return _data + _size - 1;
    }
    
    unsigned hash() const {
        if (_hash == 0 && _data) {
            const char* data = _data;
            unsigned length  = _size;
            bool remainder = length & 1;
            length >>= 1;
            _hash = stringHashingStartValue;
            
            while (length--) {
                _hash += data[0];
                _hash = (_hash << 16) ^ ((data[1] << 11) ^ _hash);
                _hash += _hash >> 11;
                data += 2;
            }
            
            if (remainder) {
                _hash += *data;
                _hash = (_hash << 16) ^ ((*data << 11) ^ _hash);
                _hash += _hash >> 11;
            }
        }
        
        return _hash;
    }
    
    void reset(const char* data, unsigned size) {
        _data = data;
        _size = size;
        _hash = 0;
    }
    
    void trim(char c) {
        if (!_data || _size == 0)
            return;
        
        unsigned size = _size;
        while(size > 0) {
            const char* p = _data + size - 1;
            if (*p == c)
                size--;
            else
                break;
        }
        
        _size = size;
        
        const char* p = _data;
        while(*p == c && _size > 1) {
            p++;
            _size--;
        }
        
        _data = p;
        _hash = 0;
    }
    
    void trim() {
        if (!_data || _size == 0)
            return;
        
        unsigned size = _size;
        while(size > 0) {
            const char* p = _data + size - 1;
            if (IS_WHITESPACE(*p))
                size--;
            else
                break;
        }
        
        _size = size;
        
        const char* p = _data;
        while (IS_WHITESPACE(*p) && _size > 1) {
            p++;
            _size--;
        }
        
        _data = p;
        _hash = 0;
    }
    
    bool startWith(const ScopedString& str) {
        if (str._size > _size)
            return false;
        
        unsigned x = 0;
        while(x < str._size) {
            if (_data[x] != str._data[x])
                return false;
            x++;
        }
        
        return true;
    }
    
    bool startWith(char c) const {
        if (_size > 0)
            return (*_data == c);
        else
            return false;
    }
    
    bool endWith(const ScopedString& str) {
        if (str._size > _size)
            return false;
        
        unsigned y = 0;
        unsigned x = _size - str._size;
        while(x < _size) {
            if (_data[x] != str._data[y])
                return false;
            x++;
            y++;
        }
        
        return true;
    }
    
    bool endWith(char c) const {
        if (_size > 0)
            return (_data[_size-1] == c);
        else
            return false;
    }
    
    int find(char c) const {
        if (!_data)
            return -1;
        
        const char* p = _data;
        for (unsigned i=0; i<_size; i++) {
            if (*p == c)
                return i;
            
            p++;
        }
        
        return -1;
    }
    
    ScopedString substr(int start) {
        int s = 0;
        if (start >= 0) {
            s += start;
        } else {
            s = _size;
            s += start;
        }
        
        if (s > 0) {
            if ((unsigned)s < _size) {
                return ScopedString(_data+s, (_size - s));
            }
        } else {
            return *this;
        }
        
        return ScopedString();
    }
    
    // hello  1:3  => ell
    // hello -1:3  => llo
    // hello  1:-1 => ello
    ScopedString substr(int start, int size) {
        int s = 0;
        int e = 0;
        
        if (start >= 0) {
            s += start;
        } else {
            s = _size;
            s += start;
        }
        
        if (size > 0) {
            e = s + size;
        } else {
            e = _size;
            e += size;
        }
        
        if (s > e) {
            s ^= e;
            e ^= s;
            s ^= e;
        }
        if (s < 0) s = 0;
        if (e > _size) e = _size;
        return ScopedString(_data + s, (e-s));
    }
    
    const char* c_str() const {
        static char buff[256];
        buff[0] = '\0';
        if (!_data)
            return buff;
        
        unsigned i;
        char* s = buff;
        const char* p = _data;
        for (i=0; i<_size && i<250; i++) {
            *s++ = *p++;
        }
        
        if (i == 250) {
            buff[250] = '.';
            buff[251] = '.';
            buff[252] = '.';
            buff[253] = '\0';
        } else {
            buff[i] = '\0';
        }
        
        return buff;
    }
    
private:
    const char*      _data;
    unsigned         _size;
    mutable unsigned _hash;
};

inline bool operator==(const ScopedString& a, const ScopedString& b) {
    if (a.size() != b.size())
        return false;
    
    const char* ap = a.data();
    const char* bp = b.data();
    
    if (ap == bp)
        return true;
    
    if (!ap || !bp)
        return false;
    
    unsigned size = a.size();
    for (unsigned i=0; i<size; i++) {
        if (*ap != *bp)
            return false;
        ap++;
        bp++;
    }
    
    return true;
}

#endif // __SCOPED_STRING_H__



// Aleksey Yevtushenko
// February 10, 2022

#if !defined(STRING_GUARD_H)
#define STRING_GUARD_H

#define STRING_INLINE inline


typedef struct Buffer{
    char *data;
    u32 count;
    u32 capacity;
}Buffer;
typedef Buffer String;

// TODO:
//
// [x] void advance(Buffer *buf, u32 count);
// [x] bool is_white(char c);
// [x] bool is_newline(char c);
// [x] bool is_spacing(char c);
// [x] bool is_alpha(char c);
// [x] bool is_digit(char c);
// [x] bool is_digit(char c);
// [x] bool is_alnum(char c);
// [x] bool is_hex(char c);
// [x] bool is_oct(char c);
// [x] bool is_bin(char c);
// [x] bool is_upper(char c);
// [x] bool is_lower(char c);
// [x] bool oneof(char c, char *s);
// [x] bool to_upper(char c);
// [x] bool to_lower(char c);
// [x] bool hex_val(char c);
// [x] bool C_comment_start(char c1, char c2);
// [x] bool C_comment_end(char c1, char c2);
// [x] bool Cpp_comment(char c1, char c2);
// [x] bool white_or_comment(char c1, char c2);
// [x] bool w32_newline(char c1, char c2);
//
// [x] void string_to_upper(char *s);
// [x] void string_to_lower(char *s);
// [x] bool string_equal(const char *s1, const char *s2);
// [x] bool string_n_equal(const char *s1, const char *s2, u32 size);
// [x] bool s_string_equal(const String s1, String s2);
// [x] bool sc_string_equal(const String s1, const char *s2);
// [x] int strnig_len(const char *s);
// [x] char *string_concat(char *s1, const char *s2);
// [] char *string_n_concat(char *s1, const char *s2);
// [x] void s_string_concat(String *s1, const String s2);
// [x] String sc_string_concat(String *s1, const char *s2);
// [x] bool string_copy(char *s1, const char *s2);
// [] bool string_n_copy(char *s1, const char *s2, u32 size);
// [x] String sc_string_copy(String *s1, const char *s2);
// [x] char *is_sub_string(const char *s1, const char *s2);

inline internal void advance(Buffer *buf, u32 count);
inline internal b32 is_white(char c);
inline internal b32 is_newline(char c);
inline internal b32 is_spacing(char c);
inline internal b32 is_alpha(char c);
inline internal b32 is_digit(char c);
inline internal b32 is_alnum(char c);
inline internal b32 is_hex(char c);
inline internal b32 is_oct(char c);
inline internal b32 is_bin(char c);
inline internal b32 is_upper(char c);
inline internal b32 is_lower(char c);
inline internal b32 oneof(char c, const char *s);
inline internal b32 to_upper(char c);
inline internal b32 to_lower(char c);
inline internal u32 hex_val(char c);
inline internal b32 C_comment_start(char c1, char c2);
inline internal b32 C_comment_end(char c1, char c2);
inline internal b32 Cpp_comment(char c1, char c2);
inline internal b32 white_or_comment(char c1, char c2);
inline internal b32 w32_newline(char c1, char c2);

inline internal void string_to_upper(char *s);
inline internal void string_to_lower(char *s);
inline internal b32 string_equal(const char *s1, const char *s2);
inline internal b32 string_n_equal(const char *s1, const char *s2, u32 size);
inline internal b32 s_string_equal(String s1, String s2);
inline internal b32 sc_string_equal(String s1, const char *s2);
inline internal b32 string_len(const char *s);
inline internal char *string_concat(char *s1, const char *s2);
inline internal char *string_n_concat(char *s1, const char *s2, u32 size);
inline internal void s_string_concat(String *s1, const String s2);
inline internal String sc_string_concat(String *s1, const char *s2);
inline internal b32 string_copy(char *s1, const char *s2);
inline internal b32 string_n_copy(char *s1, const char *s2, u32 size);
inline internal String sc_string_copy(String *s1, const char *s2);
inline internal const char *is_sub_string(const char *s1, const char *s2);

inline internal void advance(Buffer *buf, u32 count){
    if(buf->count >= count){
        buf->data += count;
        buf->count -= count;
    }
    else{
        buf->data += buf->count;
        buf->count = 0;
    }
}

inline internal b32 is_white(char c){
    return(((c == ' ') || (c == '\t') || (c == '\v') ||
            (c == '\f') || (c == '\r') || (c == '\n')));
}

inline internal b32 is_newline(char c){
    return((c == '\r') || (c == '\n'));
}

inline internal b32 is_spacing(char c){
    return(((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f')));
}

inline internal b32 is_alpha(char c){
    return((((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))));
}

inline internal b32 is_digit(char c){
    return(((c >= '0') && (c <= '9')));
}

inline internal b32 is_alnum(char c){
    return((((c >= '0') && (c <= '9')) ||
            ((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z'))));
}

inline internal b32 is_hex(char c){
    return((((c >= '0') && (c <= '9')) ||
            ((c >= 'a') && (c <= 'f')) ||
            ((c >= 'A') && (c <= 'F'))));
}

inline internal b32 is_oct(char c){
    return(((c >= '0') && (c <= '7')));
}

inline internal b32 is_bin(char c){
    return(((c == '0') || (c == '1')));
}

inline internal b32 is_upper(char c){
    return(((c >= 'A') && (c <= 'Z')));
}

inline internal b32 is_lower(char c){
    return(((c >= 'a') && (c <= 'z')));
}

inline internal b32 oneof(char c, const char *s){
    return(((c == s[0]) || (c == s[1])));
}

inline internal b32 to_upper(char c){
    b32 result = c;
    if(is_lower(c)){ result -= ('a' - 'A'); }
    return(result);
}

inline internal b32 to_lower(char c){
    b32 result = c;
    if(is_upper(c)){ result += ('a' - 'A'); }
    return((char)result);
}

inline internal u32 hex_val(char c){
    return((((c >= '0') && (c <= '9')) ? (c - '0') : (to_lower(c) - 'a' + 10)));
}

inline internal b32 C_comment_start(char c1, char c2){
    return(((c1 == '/') && (c2 == '*')));
}

inline internal b32 C_comment_end(char c1, char c2){
    return(((c1 == '*') && (c2 == '/')));
}

inline internal b32 Cpp_comment(char c1, char c2){
    return(((c1 == '/') && (c2 == '/')));
}

inline internal b32 white_or_comment(char c1, char c2){
    return((is_white(c1) || C_comment_start(c1, c2) || Cpp_comment(c1, c2)));
}

inline internal b32 w32_newline(char c1, char c2){
    return((((c1 == '\r') && (c2 == '\n')) ||
            ((c1 == '\n') && (c2 == '\r'))));
}

inline internal void string_to_upper(char *s){
    while(*s != '\0'){
        if(is_lower(*s)){ *s = to_upper(*s); }
        ++s;
    }
}

inline internal void string_to_lower(char *s){
    while(*s != '\0'){
        if(is_upper(*s)){ *s = to_lower(*s); }
        ++s;
    }
}

inline internal b32 string_equal(const char *s1, const char *s2){
    for(; *s1 == *s2; ){
        if(*s1 == '\0'){ return(0); }
        else { ++s1; ++s2; }
    }
    return(*s1 - *s2);
}

inline internal b32 string_n_equal(const char *s1, const char *s2, u32 size){
    for(s32 i = 0;
        ((*s1 == *s2) && (i < size));
        ++i){
        if((*s1 == '\0') && (i == (size - 1))){ return(0); }
        else { ++s1; ++s2; }
    }
    return(*s1 - *s2);
}

inline internal b32 sc_string_equal(String s1, const char *s2){
    if(s1.count == string_len(s2)){
        return(string_equal((const char *)s1.data, s2));
    }
    return(1);
}

inline internal b32 s_string_equal(String s1, String s2){
    if(s1.count == s2.count){
        return(string_equal((const char *)s1.data, (const char *)s2.data));
    }
    return(1);
}

inline internal b32 string_len(const char *s){
    b32 result = 0;
    s32 i;
    for(i = 0; s[i] != '\0'; ++i);
    return(i);
}

inline internal char *string_concat(char *s1, const char *s2){
    s32 i, j;
    for(i = 0; s1[i] != '\0'; ) { i++; }
    for(j = 0; s2[j] != '\0'; ) { s1[i++] = s2[j++]; }
    s1[i] = 0;
    return(s1);
}

inline internal char *string_n_concat(char *s1, const char *s2, u32 size){
    return(0);
}

inline internal void s_string_concat(String *s1, const String s2){
    if((s1->count + s2.count) > s1->capacity){
        realloc(s1->data, (s1->count + s2.count));
        s1->capacity = (s1->count + s2.count);
    }
    string_concat(s1->data, s2.data);
    s1->count += s2.count;
}

inline internal String sc_string_concat(String *s1, const char *s2){
    u32 s2_len = string_len(s2);
    if(s1->capacity >= (s1->count + s2_len)){
        s1->count += s2_len;
        string_concat(s1->data, s2);
    }
    else{
        s1->capacity = (s1->count + s2_len);
        s1->count = s1->capacity;
        realloc(s1->data, (s1->count + s2_len));
        string_concat(s1->data, s2);
    }
    return(*s1);
}

inline internal b32 string_copy(char *s1, const char *s2){
    b32 symbols_copied = 0;
    while((*s1++ = *s2++) != '\0') { ++symbols_copied; }
    return(symbols_copied);
}

inline internal b32 string_n_copy(char *s1, const char *s2, u32 size){
    return(0);
}

inline internal String sc_string_copy(String *s1, const char *s2){
    u32 s2_len = string_len(s2);
    if(s1->count >= s2_len){
        s1->count = s2_len;
        string_copy(s1->data, s2);
    }
    else{
        if(s1->capacity >= s2_len){
            s1->count = s2_len;
            string_copy(s1->data, s2);
        }
        else{
            s1->capacity = s1->count = s2_len;
            realloc(s1->data, s1->count);
            string_copy(s1->data, s2);
        }
    }
    return(*s1);
}

inline internal const char *is_sub_string(const char *s1, const char *s2){
    const char *result = s1;
    const char *str1, *str2;
    if(!*s2) return(result);
    while(*result){
        str1 = result;
        str2 = s2;
        while(*str1 && !(*str1 - *str2)){
            str1 += 1;
            str2 += 1;
        }
        if(!*str2){
            return(result);
        }
        result += 1;
    }
    return(0);
}

#endif // STRING_GUARD_H


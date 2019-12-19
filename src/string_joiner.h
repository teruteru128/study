
#include <stdio.h>
/*
    java.util.StringJoiner
    http://hg.openjdk.java.net/jdk/jdk/file/9b5bc216e922/src/java.base/share/classes/java/util/StringJoiner.java
*/
typedef struct string_joiner{
    const char* prefix;
    const char* delmiter;
    const char* suffix;
    char** elts;
    size_t size;
    size_t len;
    const char* empty_value;
}string_joiner;

/*
    string_joiner* string_joiner_create(char* delimiter, char* prefix, char* suffix);
*/ 
string_joiner* string_joiner_create(const char*, const char*, const char*);
void string_joiner_free(string_joiner*);
string_joiner* string_joiner_set_empty_value(string_joiner*, char*);
string_joiner* string_joiner_add(string_joiner*, char*);
/*
    命名案
    string_joiner_toString
    string_joiner_build
*/
char* string_joiner_toString(string_joiner*);

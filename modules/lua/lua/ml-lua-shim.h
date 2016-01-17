
#ifndef ml_lua_shim_h
#define ml_lua_shim_h

#include <dryos.h>
#include <console.h>
#include <string.h>

#define err_printf(fmt,...) console_show(), printf(fmt, ## __VA_ARGS__)
#define lua_writestringerror(s,p) (err_printf((s), (p)))
#define lua_writestring(s,l) (err_printf("%s",(s)))
#define getc(a) my_getc(a)
#define fopen(a,b) my_fopen(a,b)
#define freopen(a,b,c) my_freopen(a,b,c)
#define fclose(a) my_fclose(a)
#define ferror(a) my_ferror(a)
#define feof(a) my_feof(a)
#define fread(ptr,size,count,stream) FIO_ReadFile(stream, ptr, size * count)
#define fwrite(ptr,size,count,stream) FIO_WriteFile(stream, ptr, size * count)
#define fprintf(a,b,...) my_fprintf(a,b,__VA_ARGS__)
#define fseek(a,b,c) FIO_SeekSkipFile(a,b,c)
#define ftell(a) FIO_SeekSkipFile(a, 0, SEEK_CUR)
#define fflush(a) do_nothing()
#define setvbuf(a,b,c,d) do_nothing()
#define clearerr(a) do_nothing()
#define fgets(a,b,c) my_fgets(a,b,c)
#define tmpfile() my_tmpfile()
#define ungetc(a,b) my_ungetc(a,b)
#define strcoll(a,b) strcmp(a,b)
#define getenv(a) my_getenv(a)
#define abort() my_abort()
#define strtof(a,b) my_strtof((a),(b))
#define strpbrk(a,b) my_strpbrk((a),(b))
#define strstr(a,b) my_strstr((a),(b))

#define EOF -1
#define BUFSIZ 16384        /* allocated on stack; for lua_load_task, we have reserved 32K */
#define _IONBF 0
#define _IOFBF 0
#define _IOLBF 0

//there's no stdio
#define stdin NULL
#define stdout NULL
#define stderr NULL

//strerror doesn't work
#define strerror(a) ""

int my_getc(FILE * stream);
FILE * my_fopen(const char * filename, const char * mode);
FILE * my_freopen(const char * filename, const char * mode, FILE * f);
int my_fclose(FILE * stream);
int my_ferror(FILE * stream);
int my_feof(FILE * stream);
int do_nothing();
char * my_fgets(char * str, int num, FILE * stream );
FILE * my_tmpfile();
int my_ungetc(int character, FILE * stream);
char* my_getenv(const char* name);
void my_abort();
int ftoa(char *s, float n);
float my_strtof(const char* s, char** endptr);
char * my_strpbrk(const char * str1, const char * str2);
char * my_strstr(const char *haystack, const char *needle);

#endif
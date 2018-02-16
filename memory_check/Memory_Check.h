#ifndef Overload
#define Overload

#define UNICODE
#define _UNICODE

#define W2(x)	L##x
#define W(x)	W2(x)
#define __WFILE__	W(__FILE__)

#include <tchar.h>
#include <stdlib.h>
#include <iostream>

void *operator new (size_t size, const TCHAR *File, int Line);
void *operator new[](size_t size, const TCHAR *File, int Line);
void operator delete (void * p);
void operator delete[](void * p);

#define new new (__WFILE__, __LINE__)

#endif

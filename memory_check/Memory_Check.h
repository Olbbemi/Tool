#ifndef Overload
#define Overload

void *operator new (size_t size, const char *File, int Line);
void *operator new[](size_t size, const char *filename, int line);
void operator delete (void * p);
void operator delete[](void * p);

#define new new (__FILE__, __LINE__)

#endif

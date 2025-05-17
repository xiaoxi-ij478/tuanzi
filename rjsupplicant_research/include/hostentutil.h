#ifndef HOSTENTUTIL_H_INCLUDED
#define HOSTENTUTIL_H_INCLUDED

// written by me
void copy_hostent(struct hostent *src, struct hostent *dst);
void delete_hostent(struct hostent *entry);

#endif // HOSTENTUTIL_H_INCLUDED

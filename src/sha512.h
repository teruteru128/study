
typedef unsigned char byte;
typedef unsigned long qword;

void SHA512init(qword *buf);
void SHA512compress(qword *buf, qword *x);
void SHA512finish(qword *buf, byte *strptr, qword lswlen, qword mswlen);

#ifndef STDIO_H
#define STDIO_H

char getc(void);
void putc(char c);

coid puts(const char *s);

// This version of gets copies until newline, replacing newline with null char, or until buflen.
// Whichever comes first.
void gets(char *buf, int buflen);

#endif /* STDIO_H */
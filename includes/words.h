#ifndef WORDS_H
#define WORDS_H

int split_words(char* sentence);
void words(const char* sentence, void (*callback)(const char*, void*), void* memo);

#endif // !WORDS_H
#ifndef ANALIZOR_H
#define ANALIZOR_H

#include <stdio.h>
#include <openssl/sha.h>

void calculate_sha256(const char *dosya_yolu, char sonuc_ozet[65]);
int is_malicious(const char *dosya_hashi, const char *veritabani_yolu);
#endif

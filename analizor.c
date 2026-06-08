#include "analizor.h"
#include <string.h>

// 1. Fonksiyon: Parmak izini hesaplar
void calculate_sha256(const char *dosya_yolu, char sonuc_ozet[65]) {
    
    FILE *dosya = fopen(dosya_yolu, "rb");
    if (!dosya) {
        printf("Hata: %s dosyasi acilamadi!\n", dosya_yolu);
        return;
    }

    SHA256_CTX sha_not_defteri;
    SHA256_Init(&sha_not_defteri);
    
    unsigned char kova[4096]; 
    int okunan_bayt_sayisi = 0;

    while ((okunan_bayt_sayisi = fread(kova, 1, sizeof(kova), dosya)) != 0) {
        SHA256_Update(&sha_not_defteri, kova, okunan_bayt_sayisi);
    }

    unsigned char parmak_izi[32];
    SHA256_Final(parmak_izi, &sha_not_defteri);

    for (int i = 0; i < 32; i++) {
        sprintf(sonuc_ozet + (i * 2), "%02x", parmak_izi[i]);
    }
    sonuc_ozet[64] = '\0'; 

    fclose(dosya); 
} // <--- calculate_sha256 burada BİTTİ.

// 2. Fonksiyon: Veritabanında arama yapar
int is_malicious(const char *dosya_hashi, const char *veritabani_yolu) {
    char kayitli_hash[65];
    FILE *veritabani = fopen(veritabani_yolu, "r");
    
    if (!veritabani) {
        printf("Hata: Veritabani dosyasi acilamadi!\n");
        return 0;
    }

    while (fscanf(veritabani, "%64s", kayitli_hash) != EOF) {
        if (strcmp(dosya_hashi, kayitli_hash) == 0) {
            fclose(veritabani);
            return 1; // ZARARLI BULDUM!
        }
    }

    fclose(veritabani);
    return 0; // Tertemiz
} // <--- is_malicious burada BİTTİ.

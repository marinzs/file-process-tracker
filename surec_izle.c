/*
 * surec_izle.c
 * /proc dizinini tarayarak çalışan süreçleri listeler.
 * Önceki taramayla karşılaştırıp yeni başlayan ve biten
 * süreçleri terminale yazar.
 */

#define _DEFAULT_SOURCE /*Bu, POSIX ve GNU uzantılarını aktif eden bir makro. 
readdir gibi bazı fonksiyonlar bu tanım olmadan çalışmayabilir. 
Derleme öncesi çevreyi ayarlar.*/
#include "monitor.h"

/* /proc/<pid>/comm dosyasından süreç adını okur */
static int surec_ismi_oku(int pid, char *isim, int boyut) {
    char yol[64];
    snprintf(yol, sizeof(yol), "/proc/%d/comm", pid); 
    /*PID123 ise proc/123/comm dosyasını açarsanız içinde sürecin ismi var*/

    FILE *f = fopen(yol, "r");
    
    if (!f) return -1;

    if (fgets(isim, boyut, f) == NULL) {//boyut-1 karakter oku, isime yaz
        fclose(f);
        return -1;
    }
    fclose(f);

    /* Satır sonunu temizle */
    int uzunluk = strlen(isim);
    if (uzunluk > 0 && isim[uzunluk - 1] == '\n')
        isim[uzunluk - 1] = '\0';

    return 0;
}

/* Eski listede bu pid var mı? Liste araya benzer ama strmcmp gerektirmez çünkü tam sayı karşılaştırır */
static int pid_listede_mi(SurecKaydi liste[], int sayi, int pid) {
    int i;
    for (i = 0; i < sayi; i++) {
        if (liste[i].pid == pid)
            return 1;
    }
    return 0;
}

/* /proc'u tarar, önceki listeyle karşılaştırır */
void surec_tara(SurecKaydi onceki[], int *onceki_sayi) {

    SurecKaydi simdi[MAX_SUREC];
    int simdi_sayi = 0;
    char zaman_buf[32];
    struct tm *bilgi; 
    time_t su_an = time(NULL); //şimdiki zamanı alır

    /* Zaman damgası */
    bilgi = localtime(&su_an);
    strftime(zaman_buf, sizeof(zaman_buf), "%Y-%m-%d %H:%M:%S", bilgi);

    /* ── /proc dizinini tara ─────────────────────────────── */
    DIR *d = opendir("/proc");
    if (!d) return;

    struct dirent *giris;
    while ((giris = readdir(d)) != NULL) {

        /* Sadece sayısal klasörler PID'dir */
        int pid = atoi(giris->d_name); //proc/1234 klasörünü görünce pid=1234 
        if (pid <= 0) continue;

        char isim[256] = "?"; //programın çökmemesi için varsayılan isim
        surec_ismi_oku(pid, isim, sizeof(isim));

        simdi[simdi_sayi].pid = pid;
        strncpy(simdi[simdi_sayi].isim, isim, 255);
        simdi[simdi_sayi].isim[255] = '\0';
        simdi_sayi++;

        if (simdi_sayi >= MAX_SUREC) break;
    }
    closedir(d);

    /* ── Yeni başlayan süreçleri bul ────────────────────── */
    int i;
    for (i = 0; i < simdi_sayi; i++) {
        if (!pid_listede_mi(onceki, *onceki_sayi, simdi[i].pid)) {
            printf("%s[%s]%s %sSÜREÇ_BAŞLADI%s PID:%-6d %s\n",
                   C_DIM, zaman_buf, C_RESET,
                   C_MAGENTA, C_RESET,
                   simdi[i].pid, simdi[i].isim);
        }
    }

    /* ── Biten süreçleri bul ────────────────────────────── */
    for (i = 0; i < *onceki_sayi; i++) {
        if (!pid_listede_mi(simdi, simdi_sayi, onceki[i].pid)) {
            printf("%s[%s]%s %sSÜREÇ_BİTTİ  %s PID:%-6d %s\n",
                   C_DIM, zaman_buf, C_RESET,
                   C_RED, C_RESET,
                   onceki[i].pid, onceki[i].isim);
        }
    }

    /* ── Şimdiki listeyi bir sonraki tura aktar ─────────── */
    *onceki_sayi = simdi_sayi;
    for (i = 0; i < simdi_sayi; i++)
        onceki[i] = simdi[i];
}

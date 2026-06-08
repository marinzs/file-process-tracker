#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

/* ─── Renkler ─────────────────────────────────────────── */
#define C_RESET   "\033[0m"
#define C_RED     "\033[1;31m"
#define C_GREEN   "\033[1;32m"
#define C_YELLOW  "\033[1;33m"
#define C_CYAN    "\033[1;36m"
#define C_MAGENTA "\033[1;35m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"

/* ─── Sabitler ────────────────────────────────────────── */
#define MAX_DOSYA     1024   /* en fazla takip edilecek dosya sayısı */
#define MAX_SUREC     1024   /* en fazla takip edilecek süreç sayısı */
#define TARAMA_ARALIGI 2     /* kaç saniyede bir taransın */

/* ─── Dosya kaydı yapısı ──────────────────────────────── */
typedef struct {
    char yol[512];           /* dosyanın tam yolu */
    time_t son_degisim;      /* son değiştirilme zamanı */
    int var_mi;              /* hâlâ var mı? */
} DosyaKaydi;

/* ─── Süreç kaydı yapısı ──────────────────────────────── */
typedef struct {
    int pid;
    char isim[256];
} SurecKaydi;

/* ─── Fonksiyon prototipleri ──────────────────────────── */

/* dosya_izle.c */
void dizin_tara(const char *dizin, DosyaKaydi onceki[], int *onceki_sayi,
                FILE *log_dosyasi);

/* surec_izle.c */
void surec_tara(SurecKaydi onceki[], int *onceki_sayi);

#endif

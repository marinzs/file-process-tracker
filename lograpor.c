/*
 * lograpor.c  –  3. arkadaşın kodu
 *
 * KISIM 1: nmap ile ağı tarar, açık portları nmap_log.txt'ye yazar
 * KISIM 2: uyarilar.txt'yi okur, av_report.txt'ye güzel formatta yazar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define NMAP_LOG      "nmap_log.txt"
#define AV_REPORT     "av_report.txt"
#define UYARILAR      "uyarilar.txt"   /* 2. arkadaşın yazdığı dosya */
#define NMAP_TARGET   "192.168.1.0/24"
#define MAX_LINE      512

/* Zaman damgası üretir */
static void zaman_al(char *buf, int boyut) {
    time_t t = time(NULL);
    strftime(buf, boyut, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

/* ═══════════════════════════════════════════════
   KISIM 1 — NMAP TARAMA & LOGLAMA
   ═══════════════════════════════════════════════ */
void run_nmap_scan(void) {
    char ts[32];
    zaman_al(ts, sizeof(ts));

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "nmap -sV --open -T4 %s 2>/dev/null", NMAP_TARGET);

    FILE *log = fopen(NMAP_LOG, "a");
    if (!log) {
        perror("[HATA] nmap_log.txt acilamadi");
        return;
    }

    fprintf(log, "\n╔══════════════════════════════════╗\n");
    fprintf(log, "║  TARAMA ZAMANI : %-16s  ║\n", ts);
    fprintf(log, "║  HEDEF         : %-16s  ║\n", NMAP_TARGET);
    fprintf(log, "╚══════════════════════════════════╝\n");

    FILE *pipe_fp = popen(cmd, "r");
    if (!pipe_fp) {
        perror("[HATA] nmap baslatilamadi (kurulu mu?)");
        fclose(log);
        return;
    }

    char line[MAX_LINE];
    int open_count = 0;

    while (fgets(line, sizeof(line), pipe_fp)) {
        if (strstr(line, "open")) {
            fprintf(log, "  [ACIK PORT] %s", line);
            open_count++;
        } else if (strstr(line, "Nmap scan report")) {
            fprintf(log, "\n%s", line);
        }
    }

    fprintf(log, "\n  → Toplam acik port: %d\n", open_count);

    pclose(pipe_fp);
    fclose(log);

    printf("[%s] Nmap taramasi bitti. Acik port: %d → %s\n",
           ts, open_count, NMAP_LOG);
}

/* ═══════════════════════════════════════════════
   KISIM 2 — UYARI RAPORLAMA
   ═══════════════════════════════════════════════ */

/* Tek bir zararlıyı av_report.txt'ye yazar */
static void report_threat(const char *filepath, const char *action,
                           const char *severity) {
    char ts[32];
    zaman_al(ts, sizeof(ts));

    FILE *rep = fopen(AV_REPORT, "a");
    if (!rep) {
        perror("[HATA] av_report.txt acilamadi");
        return;
    }

    fprintf(rep, "%s | SEVIYE:%-6s | ZARARLI: %-30s | Islem: %s\n",
            ts, severity, filepath, action);
    fclose(rep);

    printf("\033[1;31m[%s] ZARARLI: %s → %s (%s)\033[0m\n",
           ts, filepath, action, severity);
}

/*
 * uyarilar.txt'yi okur.
 * 2. arkadaşın yazdığı format: ZARARLI|dosya_yolu|hash
 * Biz bunu okuyup av_report.txt'ye yazıyoruz.
 */
void check_av_notifications(void) {
    FILE *f = fopen(UYARILAR, "r");
    if (!f) return; /* dosya yoksa bildirim yok, normal */

    char line[MAX_LINE];
    int count = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        /* Format: ZARARLI|dosya_yolu|hash */
        char *durum   = strtok(line, "|");
        char *filepath = strtok(NULL, "|");
        char *hash     = strtok(NULL, "|");

        if (durum && filepath && hash) {
            /* Severity ve action sabit koyuyoruz,
               ileride geliştirilebilir */
            report_threat(filepath, "Karantina", "YUKSEK");
            count++;
        }
    }

    fclose(f);

    if (count > 0)
        printf("[BILGI] %d zararli bildirimi islendi → %s\n",
               count, AV_REPORT);
}

/* ═══════════════════════════════════════════════
   KISIM 3 — DURUM ÖZETİ
   ═══════════════════════════════════════════════ */
void print_status_summary(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════╗\n");
    printf("║      DURUM OZETI — AG GUVENLİGİ       ║\n");
    printf("╚═══════════════════════════════════════╝\n");

    printf("\n[ Son Nmap Bulgulari ]\n");
    FILE *fp = popen("tail -20 " NMAP_LOG " 2>/dev/null | grep 'ACIK PORT'", "r");
    if (fp) {
        char line[MAX_LINE];
        int shown = 0;
        while (fgets(line, sizeof(line), fp) && shown < 5) {
            printf("  %s", line);
            shown++;
        }
        if (shown == 0) printf("  (henuz veri yok)\n");
        pclose(fp);
    }

    printf("\n[ Son Zararli Tespitleri ]\n");
    fp = popen("tail -3 " AV_REPORT " 2>/dev/null", "r");
    if (fp) {
        char line[MAX_LINE];
        int shown = 0;
        while (fgets(line, sizeof(line), fp)) {
            printf("  %s", line);
            shown++;
        }
        if (shown == 0) printf("  (henuz kayit yok)\n");
        pclose(fp);
    }

    printf("\n");
}

/* ═══════════════════════════════════════════════
   ANA FONKSİYON — main.c'den çağrılır
   ═══════════════════════════════════════════════ */
void adim3_log_analizi(void) {
    printf("\n");
    printf("\033[1;36m╔══════════════════════════════════════════╗\n\033[0m");
    printf("\033[1;36m║\033[0m\033[1m     ADIM 3: LOG & AG ANALİZİ             \033[0m\033[1;36m║\n\033[0m");
    printf("\033[1;36m╚══════════════════════════════════════════╝\n\n\033[0m");

    /* 2. arkadaştan gelen uyarıları işle */
    check_av_notifications();

    /* Ağ taraması yap */
    printf("[*] Ag taramasi baslatiliyor (%s)...\n", NMAP_TARGET);
    run_nmap_scan();

    /* Özet göster */
    print_status_summary();
}

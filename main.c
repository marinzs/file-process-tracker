/*
 * main.c  –  Güvenlik Analiz Aracı
 *
 * Kullanım: sudo ./monitor [dizin] [sure] [zararli_db]
 * Örnek   : sudo ./monitor /home/selenay/Masaustu 30 zararli.txt
 *
 * Akış:
 *   ADIM 1 → dizini ve süreçleri izle, olaylar.txt yaz
 *   ADIM 2 → olaylar.txt'deki dosyaların hash'ini kontrol et
 *   ADIM 3 → 3. log analizi
 */

#include "monitor.h"
#include "analizor.h"
#include "lograpor.h"

/* ─── Banner ──────────────────────────────────────────── */
static void banner_yazdir(const char *dizin, int sure) {
    printf("\n");
    printf(C_CYAN "╔══════════════════════════════════════════╗\n" C_RESET);
    printf(C_CYAN "║" C_BOLD "     GÜVENLİK ANALİZ ARACI  v3.0          " C_RESET C_CYAN "║\n" C_RESET);
    printf(C_CYAN "╠══════════════════════════════════════════╣\n" C_RESET);
    printf(C_CYAN "║" C_RESET "  Adım 1 : Sistem İzleme                  " C_CYAN "║\n" C_RESET);
    printf(C_CYAN "║" C_RESET "  Adım 2 : Hash Analizi                   " C_CYAN "║\n" C_RESET);
    printf(C_CYAN "║" C_RESET "  Adım 3 : Log Analizi      " C_DIM "(yakında)" C_RESET "     " C_CYAN "║\n" C_RESET);
    printf(C_CYAN "╠══════════════════════════════════════════╣\n" C_RESET);
    printf(C_CYAN "║" C_RESET "  Dizin  : " C_GREEN "%-31s" C_RESET C_CYAN "║\n" C_RESET, dizin);
    printf(C_CYAN "║" C_RESET "  Süre   : " C_YELLOW "%-3d saniye" C_RESET "                      " C_CYAN "║\n" C_RESET, sure);
    printf(C_CYAN "╚══════════════════════════════════════════╝\n\n" C_RESET);
}

/* ════════════════════════════════════════════════════════
 *  ADIM 2 — Hash Analizi
 * ════════════════════════════════════════════════════════ */
static void adim2_hash_analizi(const char *db_yolu) {
    printf("\n");
    printf(C_CYAN "╔══════════════════════════════════════════╗\n" C_RESET);
    printf(C_CYAN "║" C_BOLD "     ADIM 2: HASH ANALİZİ                 " C_RESET C_CYAN "║\n" C_RESET);
    printf(C_CYAN "╚══════════════════════════════════════════╝\n\n" C_RESET);

    FILE *olaylar = fopen("olaylar.txt", "r");
    if (!olaylar) {
        printf(C_RED "[!] olaylar.txt bulunamadı!\n" C_RESET);
        return;
    }

    FILE *uyarilar = fopen("uyarilar.txt", "w");
    if (!uyarilar) {
        printf(C_RED "[!] uyarilar.txt oluşturulamadı!\n" C_RESET);
        fclose(olaylar);
        return;
    }

    char dosya_yolu[512];
    int temiz = 0, zararli = 0, toplam = 0;

    while (fgets(dosya_yolu, sizeof(dosya_yolu), olaylar)) {
        /* Satır sonunu temizle */
        dosya_yolu[strcspn(dosya_yolu, "\n")] = '\0';
        if (strlen(dosya_yolu) == 0) continue;

        toplam++;
        char hash[65];
        calculate_sha256(dosya_yolu, hash);

        int kotumu = is_malicious(hash, db_yolu);

        if (kotumu) {
            zararli++;
            printf(C_RED "[ZARARLI] %s\n         Hash: %s\n" C_RESET,
                   dosya_yolu, hash);
            fprintf(uyarilar, "ZARARLI|%s|%s\n", dosya_yolu, hash);
        } else {
            temiz++;
            printf(C_GREEN "[TEMİZ]  %s\n" C_RESET, dosya_yolu);
        }
    }

    fclose(olaylar);
    fclose(uyarilar);

    printf("\n" C_BOLD "── Analiz Sonucu ──────────────────────────\n" C_RESET);
    printf("  Toplam : %d\n", toplam);
    printf(C_GREEN "  Temiz  : %d\n" C_RESET, temiz);
    printf(C_RED   "  Zararlı: %d\n" C_RESET, zararli);
    if (zararli > 0)
        printf(C_YELLOW "  → uyarilar.txt oluşturuldu.\n" C_RESET);
}

/* ════════════════════════════════════════════════════════
 *  MAIN
 * ════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {

    /* Komut satırı argümanları */
    const char *dizin   = (argc >= 2) ? argv[1] : "/tmp";
    int         sure    = (argc >= 3) ? atoi(argv[2]) : 30;
    const char *db_yolu = (argc >= 4) ? argv[3] : "zararli.txt";

    banner_yazdir(dizin, sure);

    /* ══ ADIM 1: İzleme ═══════════════════════════════════ */
    printf(C_BOLD "── ADIM 1: Sistem İzleniyor (%d saniye) ───\n\n" C_RESET, sure);

    /* Log dosyasını aç */
    FILE *log_dosyasi = fopen("olaylar.txt", "w");
    if (!log_dosyasi) {
        printf(C_RED "[!] olaylar.txt açılamadı!\n" C_RESET);
        return 1;
    }

    /* Önceki durum listeleri */
    DosyaKaydi onceki_dosyalar[MAX_DOSYA];
    int onceki_dosya_sayi = 0;

    SurecKaydi onceki_surecler[MAX_SUREC];
    int onceki_surec_sayi = 0;

    /* İlk taramayı yap (başlangıç durumunu kaydet) */
    dizin_tara(dizin, onceki_dosyalar, &onceki_dosya_sayi, NULL);
    surec_tara(onceki_surecler, &onceki_surec_sayi);

    /* Kaç tur tarama yapacağımızı hesapla */
    int tur_sayisi = sure / TARAMA_ARALIGI;
    int tur;

    for (tur = 0; tur < tur_sayisi; tur++) {
        sleep(TARAMA_ARALIGI);
        dizin_tara(dizin, onceki_dosyalar, &onceki_dosya_sayi, log_dosyasi);
        surec_tara(onceki_surecler, &onceki_surec_sayi);
    }

    fclose(log_dosyasi);
    printf(C_GREEN "\n[✓] İzleme tamamlandı → olaylar.txt oluşturuldu.\n" C_RESET);

    /* ══ ADIM 2: Hash Analizi ═════════════════════════════ */
    adim2_hash_analizi(db_yolu);

    /* ══ ADIM 3: Log & Ağ Analizi ════════════════════════ */
    adim3_log_analizi();

    printf(C_GREEN "\n[✓] Tüm adımlar tamamlandı.\n" C_RESET);
    return 0;
}

/*
 * dosya_izle.c
 * Verilen dizini tarayıp önceki durumla karşılaştırır.
 * Yeni dosya, silinen dosya veya değişen dosyayı terminale yazar
 * ve log dosyasına kaydeder.
 */

#include "monitor.h"

/* Zaman damgasını okunabilir formata çevirir */
static void zaman_yazdir(time_t t, char *buf, int buf_boyut) {
    struct tm *bilgi = localtime(&t);
    strftime(buf, buf_boyut, "%Y-%m-%d %H:%M:%S", bilgi);
}

/* Eski listede bu yol var mı? Varsa indeksini döndürür, yoksa -1 */
static int listede_ara(DosyaKaydi liste[], int sayi, const char *yol) {
    int i;
    for (i = 0; i < sayi; i++) {
        if (strcmp(liste[i].yol, yol) == 0)
            return i;
    }
    return -1;
}

/* Dizini tarar, önceki snapshot ile karşılaştırır */
void dizin_tara(const char *dizin, DosyaKaydi onceki[], int *onceki_sayi,FILE *log_dosyasi) {
    /* FILE dosya açmak için gereken karmaşıklığı gizler. Dosya akışını temsil eder.*/

    DosyaKaydi simdi[MAX_DOSYA]; //şuan klasörde hangi dosyalar var onları tutacak array
    int simdi_sayi = 0;

    char zaman_buf[32]; //zaman yazdır fonksiyonunun sonucunu tutacak buffer
    char tam_yol[512]; //dizin + dosya adı için tam yol oluşturmak için kullanılacak buffer
    struct stat dosya_bilgi; //stat fonksiyonu ile dosya bilgilerini tutacak struct. Lİnuxa özel..

    /* ── Dizini oku, şu anki durumu al ─────────────────── */
    DIR *d = opendir(dizin); /*DIR bir klasör akışını temsil eder ve Unixte çalışır*/
    if (!d) {
        printf(C_RED "[!] Dizin açılamadı: %s\n" C_RESET, dizin);
        return;
    }

    struct dirent *giris; //Dirent dosya adı gibi bilgileri alır. Lİnuxa özel.
    while ((giris = readdir(d)) != NULL) { 
        /* readdir her çağrıda bir sonraki girdiyi döndürür,
         dizi bitincec Null döndürür. Linuxa özel.*/

        /* Linuxta her klasörün içindeki gizli girdileri atla */
        if (strcmp(giris->d_name, ".") == 0 || strcmp(giris->d_name, "..") == 0)
            continue;

        /* Tam yolu oluştur: dizin + "/" + dosya_adı */
        snprintf(tam_yol, sizeof(tam_yol), "%s/%s", dizin, giris->d_name);

        /* Dosya bilgilerini al */
        if (stat(tam_yol, &dosya_bilgi) != 0) /*stat() linux sistem çağrısı. 
        Dosya hakkındaki metadatayı dosya bilgi structına doldurur
        başarısız olursa sıfır dışı değer döner; atla*/
            continue;

        /* Dizinleri atla, sadece dosyaları izle */
        if (S_ISDIR(dosya_bilgi.st_mode)) /*st_mode dan bitlere bakıp bir dizin mi diye soruyor. bir makro*/
            continue;

        /* Şimdiki listeye ekle */
        strncpy(simdi[simdi_sayi].yol, tam_yol, 511); 
        //strcnpy(hedef,kaynak,kopyalancak max karakter sayısı)
        simdi[simdi_sayi].yol[511] = '\0'; //son byte manuel olarak null yapıyoruz
        simdi[simdi_sayi].son_degisim = dosya_bilgi.st_mtime; //son değiştirilme zamanını kaydediyoruz
        simdi[simdi_sayi].var_mi = 1;
        simdi_sayi++;

        if (simdi_sayi >= MAX_DOSYA) break; //1024 e ulaşırsak dur
    }
    closedir(d); //dizi akışını kapar

    /* ── Yeni veya değişen dosyaları bul ────────────────── */
    int i;
    for (i = 0; i < simdi_sayi; i++) {
        int indeks = listede_ara(onceki, *onceki_sayi, simdi[i].yol); //önceki listede var mı
        zaman_yazdir(time(NULL), zaman_buf, sizeof(zaman_buf)); //zaman damgasını hazırla

        if (indeks == -1) {
            /* Önceki listede yok → YENİ dosya */
            printf("%s[%s]%s %sYENİ_DOSYA   %s %s\n",
                   C_DIM, zaman_buf, C_RESET,
                   C_GREEN, C_RESET, simdi[i].yol);

            if (log_dosyasi) {
                fprintf(log_dosyasi, "%s\n", simdi[i].yol);//dosyaya yaz
                fflush(log_dosyasi); //bufferdakileri hemen diske yaz, böylece program kapanırsa kaybolmaz
            }

        } else if (onceki[indeks].son_degisim != simdi[i].son_degisim) {
            /* Önceki listede var ama zamanı değişmiş → DEĞİŞTİ */
            printf("%s[%s]%s %sDEĞİŞTİ     %s %s\n",
                   C_DIM, zaman_buf, C_RESET,
                   C_YELLOW, C_RESET, simdi[i].yol);

            if (log_dosyasi) {
                fprintf(log_dosyasi, "%s\n", simdi[i].yol); //dosyaya yaz
                fflush(log_dosyasi); //bufferdakileri hemen diske yaz
            }
        }
    }

    /* ── Silinen dosyaları bul ───────────────────────────── */
    for (i = 0; i < *onceki_sayi; i++) {
        if (listede_ara(simdi, simdi_sayi, onceki[i].yol) == -1) {
            zaman_yazdir(time(NULL), zaman_buf, sizeof(zaman_buf));
            printf("%s[%s]%s %sSİLİNDİ     %s %s\n",
                   C_DIM, zaman_buf, C_RESET,
                   C_RED, C_RESET, onceki[i].yol);
        }
    }

    /* ── Şimdiki listeyi bir sonraki tura aktar ─────────── */
    *onceki_sayi = simdi_sayi;
    for (i = 0; i < simdi_sayi; i++)
        onceki[i] = simdi[i];
}

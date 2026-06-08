CC     = gcc
CFLAGS = -Wall -g
LIBS   = -lssl -lcrypto

DOSYALAR = main.c dosya_izle.c surec_izle.c analizor.c lograpor.c

monitor: $(DOSYALAR)
	$(CC) $(CFLAGS) -o monitor $(DOSYALAR) $(LIBS)
	@echo ""
	@echo "  Derleme tamamlandi!"
	@echo "  Kullanim: sudo ./monitor [dizin] [sure] [zararli_db]"
	@echo "  Ornek   : sudo ./monitor /home/selenay/Masaustu 30 zararli.txt"
	@echo ""

clean:
	rm -f monitor

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 15000 // büyük verilerde denemek için arttırılmıs kapasite
#define INF 999999

// 1. GRAF KENAR YAPISI
typedef struct Edge {
    char hedef_durak_id[20];
    int sure;
    char vasita_turu[30];
    struct Edge* next;
} Edge;

//  2. DURAK YAPISI (Hash Table & Graf Düğümü)
typedef struct Station {
    char id[20];
    char ad[50];
    char gecen_hatlar[10][20];
    int hat_sayisi;
    Edge* komsular;
    struct Station* next;

    // Dijkstra Algoritması Değişkenleri
    int mesafe;
    int ziyaret_edildi;
    char onceki_durak_id[20];
    char kullanilan_vasita[30];
} Station;

// HASH TABLE
Station* durak_tablosu[TABLE_SIZE];

int hash_fonksiyonu(char* id) {
    int toplam = 0;
    for (int i = 0; id[i] != '\0'; i++) toplam += id[i];
    return toplam % TABLE_SIZE;
}

Station* durak_olustur(char* id, char* ad) {
    Station* yeni = (Station*)malloc(sizeof(Station));
    strcpy(yeni->id, id);
    strcpy(yeni->ad, ad);
    yeni->hat_sayisi = 0;
    yeni->komsular = NULL;
    yeni->next = NULL;
    return yeni;
}

void durak_ekle(char* id, char* ad) {
    int index = hash_fonksiyonu(id);
    Station* yeni = durak_olustur(id, ad);

    if (durak_tablosu[index] == NULL) {
        durak_tablosu[index] = yeni;
    } else {
        Station* temp = durak_tablosu[index];
        while (temp->next != NULL) temp = temp->next;
        temp->next = yeni;
    }
}

Station* durak_bul(char* id) {
    int index = hash_fonksiyonu(id);
    Station* temp = durak_tablosu[index];
    while (temp != NULL) {
        if (strcmp(temp->id, id) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void duraga_hat_ekle(char* durak_id, char* hat_adi) {
    Station* durak = durak_bul(durak_id);
    if (durak == NULL) return;
    if (durak->hat_sayisi >= 10) return;

    for (int i = 0; i < durak->hat_sayisi; i++) {
        if (strcmp(durak->gecen_hatlar[i], hat_adi) == 0) return;
    }
    strcpy(durak->gecen_hatlar[durak->hat_sayisi], hat_adi);
    durak->hat_sayisi++;
}

void baglanti_ekle(char* kaynak_id, char* hedef_id, int sure, char* vasita_turu) {
    Station* kaynak = durak_bul(kaynak_id);
    Station* hedef = durak_bul(hedef_id);

    if (kaynak != NULL && hedef != NULL) {
        Edge* yeni_baglanti = (Edge*)malloc(sizeof(Edge));
        strcpy(yeni_baglanti->hedef_durak_id, hedef_id);
        yeni_baglanti->sure = sure;
        strcpy(yeni_baglanti->vasita_turu, vasita_turu);

        yeni_baglanti->next = kaynak->komsular;
        kaynak->komsular = yeni_baglanti;
    }
}

// 3. GÜNCELLEME İŞLEMİ
void baglanti_guncelle(char* kaynak_id, char* hedef_id, char* vasita_turu, int yeni_sure) {
    Station* kaynak = durak_bul(kaynak_id);
    if (kaynak == NULL) {
        printf("Hata: Kaynak durak bulunamadi.\n");
        return;
    }

    Edge* temp = kaynak->komsular;
    int bulundu = 0;
    while (temp != NULL) {
        if (strcmp(temp->hedef_durak_id, hedef_id) == 0 && strcmp(temp->vasita_turu, vasita_turu) == 0) {
            temp->sure = yeni_sure;
            bulundu = 1;
            printf("[BASARILI] Rota suresi %d dakika olarak guncellendi.\n", yeni_sure);
            break;
        }
        temp = temp->next;
    }
    if (!bulundu) printf("Hata: Baglanti bulunamadi.\n");
}

// 4. SİLME İŞLEMİ (Graf ve Bellek Temizliği)
void durak_sil(char* id) {
    int index = hash_fonksiyonu(id);
    Station* temp = durak_tablosu[index];
    Station* onceki = NULL;

    while (temp != NULL && strcmp(temp->id, id) != 0) {
        onceki = temp;
        temp = temp->next;
    }
    if (temp == NULL) {
        printf("Hata: Durak bulunamadi!\n");
        return;
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        Station* s = durak_tablosu[i];
        while (s != NULL) {
            Edge* e_temp = s->komsular;
            Edge* e_onceki = NULL;
            while (e_temp != NULL) {
                if (strcmp(e_temp->hedef_durak_id, id) == 0) {
                    if (e_onceki == NULL) s->komsular = e_temp->next;
                    else e_onceki->next = e_temp->next;

                    Edge* silinecek = e_temp;
                    e_temp = e_temp->next;
                    free(silinecek);
                } else {
                    e_onceki = e_temp;
                    e_temp = e_temp->next;
                }
            }
            s = s->next;
        }
    }

    Edge* kendi_kenari = temp->komsular;
    while (kendi_kenari != NULL) {
        Edge* sil = kendi_kenari;
        kendi_kenari = kendi_kenari->next;
        free(sil);
    }

    if (onceki == NULL) durak_tablosu[index] = temp->next;
    else onceki->next = temp->next;

    free(temp);
    printf("[BASARILI] %s ID'li durak sistemden kalici olarak silindi.\n", id);
}

//  DOSYA OKUMA
void duraklari_yukle(char* dosya_adi) {
    FILE *dosya = fopen(dosya_adi, "r");
    if (dosya == NULL) return;
    char satir[256];
    int sayac = 0;
    while (fgets(satir, sizeof(satir), dosya)) {
        char *id = strtok(satir, ",\n\r");
        char *ad = strtok(NULL, ",\n\r");
        if (id != NULL && ad != NULL) {
            durak_ekle(id, ad);
            sayac++;
        }
    }
    fclose(dosya);
    printf("[SISTEM] %d adet durak yuklendi.\n", sayac);
}

void baglantilari_yukle(char* dosya_adi) {
    FILE *dosya = fopen(dosya_adi, "r");
    if (dosya == NULL) return;
    char satir[256];
    int sayac = 0;
    while (fgets(satir, sizeof(satir), dosya)) {
        char *kaynak = strtok(satir, ",\n\r");
        char *hedef = strtok(NULL, ",\n\r");
        char *sure_str = strtok(NULL, ",\n\r");
        char *vasita = strtok(NULL, ",\n\r");

        if (kaynak != NULL && hedef != NULL && sure_str != NULL && vasita != NULL) {
            int sure = atoi(sure_str);
            baglanti_ekle(kaynak, hedef, sure, vasita);
            duraga_hat_ekle(kaynak, vasita);
            duraga_hat_ekle(hedef, vasita);
            sayac++;
        }
    }
    fclose(dosya);
    printf("[SISTEM] %d adet baglanti yuklendi.\n", sayac);
}

//  MIN-HEAP (ÖNCELİKLİ KUYRUK)
typedef struct MinHeapNode {
    char durak_id[20];
    int mesafe;
} MinHeapNode;

MinHeapNode heap[100000];
int heap_boyutu = 0;

void heap_ekle(char* id, int mesafe) {
    int i = heap_boyutu++;
    heap[i].mesafe = mesafe;
    strcpy(heap[i].durak_id, id);
    while (i != 0 && heap[(i - 1) / 2].mesafe > heap[i].mesafe) {
        MinHeapNode temp = heap[i];
        heap[i] = heap[(i - 1) / 2];
        heap[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

MinHeapNode heap_cikar() {
    if (heap_boyutu <= 0) { MinHeapNode bos = {"", -1}; return bos; }
    if (heap_boyutu == 1) { heap_boyutu--; return heap[0]; }
    MinHeapNode root = heap[0];
    heap[0] = heap[heap_boyutu - 1];
    heap_boyutu--;
    int i = 0;
    while (1) {
        int sol = 2 * i + 1, sag = 2 * i + 2, en_kucuk = i;
        if (sol < heap_boyutu && heap[sol].mesafe < heap[en_kucuk].mesafe) en_kucuk = sol;
        if (sag < heap_boyutu && heap[sag].mesafe < heap[en_kucuk].mesafe) en_kucuk = sag;
        if (en_kucuk != i) {
            MinHeapNode temp = heap[i];
            heap[i] = heap[en_kucuk];
            heap[en_kucuk] = temp;
            i = en_kucuk;
        } else break;
    }
    return root;
}

// stack yapisi
// rotayı düzeltmek için lifo yapisi
typedef struct StackNode {
    char adim[100];
    struct StackNode* next;
} StackNode;

StackNode* stack_tepe = NULL;

void stack_push(char* metin) {
    StackNode* yeni = (StackNode*)malloc(sizeof(StackNode));
    strcpy(yeni->adim, metin);
    yeni->next = stack_tepe;
    stack_tepe = yeni;
}

int stack_pop(char* hedef) {
    if (stack_tepe == NULL) return 0;
    StackNode* silinecek = stack_tepe;
    strcpy(hedef, silinecek->adim);
    stack_tepe = stack_tepe->next;
    free(silinecek);
    return 1;
}

// DIJKSTRA ALGORİTMASI
void en_kisa_rotayi_bul(char* baslangic_id, char* hedef_id) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Station* temp = durak_tablosu[i];
        while (temp != NULL) {
            temp->mesafe = INF;
            temp->ziyaret_edildi = 0;
            strcpy(temp->onceki_durak_id, "");
            temp = temp->next;
        }
    }

    Station* baslangic = durak_bul(baslangic_id);
    Station* hedef = durak_bul(hedef_id);

    if (baslangic == NULL || hedef == NULL) {
        printf("Hata: Baslangic veya hedef durak bulunamadi!\n");
        return;
    }

    baslangic->mesafe = 0;
    heap_boyutu = 0;
    heap_ekle(baslangic_id, 0);

    while (heap_boyutu > 0) {
        MinHeapNode min_node = heap_cikar();
        Station* u = durak_bul(min_node.durak_id);

        if (u->ziyaret_edildi) continue;
        u->ziyaret_edildi = 1;
        if (strcmp(u->id, hedef_id) == 0) break;

        Edge* komsu_kenar = u->komsular;
        while (komsu_kenar != NULL) {
            Station* v = durak_bul(komsu_kenar->hedef_durak_id);
            if (!v->ziyaret_edildi && u->mesafe + komsu_kenar->sure < v->mesafe) {
                v->mesafe = u->mesafe + komsu_kenar->sure;
                strcpy(v->onceki_durak_id, u->id);
                strcpy(v->kullanilan_vasita, komsu_kenar->vasita_turu);
                heap_ekle(v->id, v->mesafe);
            }
            komsu_kenar = komsu_kenar->next;
        }
    }

    if (hedef->mesafe == INF) {
        printf("\nBaglanti bulunamadi!\n");
    } else {
        printf("\n EN KISA ROTA\n");
        printf("Toplam Seyahat Suresi: %d Dakika\n", hedef->mesafe);

        // Yığın kullanarak rotayı düzeltme
        Station* temp = hedef;
        while (strcmp(temp->id, baslangic_id) != 0) {
            char adim_metni[100];
            sprintf(adim_metni, "(%s) -> %s", temp->kullanilan_vasita, temp->ad);
            stack_push(adim_metni); // Son giren (LIFO)
            temp = durak_bul(temp->onceki_durak_id);
        }

        printf("Guzergah: %s ", baslangic->ad);
        char cikan_adim[100];
        while (stack_pop(cikan_adim)) { // İlk çıkar (LIFO)
            printf("%s ", cikan_adim);
        }
        printf("\n");
    }
}

int main() {
    for(int i = 0; i < TABLE_SIZE; i++) durak_tablosu[i] = NULL;
    printf(" SEHIR ICI TOPLU TASIMA ANALIZ SISTEMI \n\n");
    duraklari_yukle("duraklar.txt");
    baglantilari_yukle("baglantilar.txt");

    int secim;
    char baslangic[20], hedef[20], sorgu_id[20];

    while (1) {
        printf("\n ISLEM MENUSU \n");
        printf("1. Durak Bilgisi Sorgula (Hash Table & Set)\n");
        printf("2. En Kisa Rotayi Bul (Min-Heap, Dijkstra & Stack)\n");
        printf("3. Baglanti Suresi Guncelle\n");
        printf("4. Durak Sil (Graf Temizligi)\n");
        printf("5. Cikis\n");
        printf("Seciminiz: ");
        scanf("%d", &secim);

        if (secim == 1) {
            printf("Durak ID: "); scanf("%s", sorgu_id);
            Station* bulunan = durak_bul(sorgu_id);
            if (bulunan != NULL) {
                printf("\nID: %s\nAd: %s\nGeçen Hatlar: ", bulunan->id, bulunan->ad);
                for (int i = 0; i < bulunan->hat_sayisi; i++) printf("%s ", bulunan->gecen_hatlar[i]);
                printf("\n");
            } else printf("Durak bulunamadi!\n");
        }
        else if (secim == 2) {
            printf("Baslangic Durak ID: "); scanf("%s", baslangic);
            printf("Hedef Durak ID: "); scanf("%s", hedef);
            clock_t baslangic_zamani = clock();
            en_kisa_rotayi_bul(baslangic, hedef);
            clock_t bitis_zamani = clock();
            printf("\n[PERFORMANS] Hesaplama Suresi: %.4f milisaniye\n\n",
                   ((double)(bitis_zamani - baslangic_zamani) / CLOCKS_PER_SEC) * 1000.0);
        }
        else if (secim == 3) {
            char vasita[30]; int yeni_sure;
            printf("Kaynak ID: "); scanf("%s", baslangic);
            printf("Hedef ID: "); scanf("%s", hedef);
            printf("Vasita Turu: "); scanf("%s", vasita);
            printf("Yeni Sure: "); scanf("%d", &yeni_sure);
            baglanti_guncelle(baslangic, hedef, vasita, yeni_sure);
        }
        else if (secim == 4) {
            printf("Silinecek Durak ID: "); scanf("%s", sorgu_id);
            durak_sil(sorgu_id);
        }
        else if (secim == 5) {
            printf("Sistemden cikiliyor...\n");
            break;
        }
        else printf("Gecersiz secim!\n");
    }
    return 0;
}
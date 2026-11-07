#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define _N "\033[0m" //reset
#define _NG "\033[32m" //normal-green
#define _NY "\033[33m" //normal-yellow
#define _NR "\033[31m" //normal-red
#define _BG "\033[1;32m" //bold-green
#define _BY "\033[1;33m" //bold-yellow
#define _BB "\033[1;34m" //bold-blue
#define _IY "\033[3;33m" //italic-yellow

#ifdef _WIN32
#include <windows.h>
#define clrscr() system("cls")
#define my_sleep(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#define clrscr() system("clear")
#define my_sleep(seconds) sleep(seconds)
#endif

// >>> Struktur data
typedef struct {
    char nama[26];
    int jkel;
    char usr[9];
    char psw[9];
    char kdusr[5];
} User;

typedef struct {
    int umur;
    float berat;
    int tinggi;
    char kdaktiv[4];
    char tgl[9];
    char kdusr[5];
} History;

typedef struct {
    char aktivitas[35];
    char kdaktiv[4];
} Aktivitas;
// <<< Struktur data

// >>> Variabel/Konstanta (Global)
char* FlUser="users.txt";  //nama file data untuk users
char* FlHistory="histories.txt";  //nama file data untuk history
char* FlUser2="users.bak";  //nama file data untuk users (backup)
char* FlHistory2="histories.bak";  //nama file data untuk history (backup)
int JmlDtUser, JmlDtHistory; //jumlah data untuk masing-masing array
int BufferDtUser=50; //jumlah array kosong untuk inputan data users baru
int BufferDtHistory=100; //jumlah array kosong untuk inputan data histories baru (2x jumlah users)
int KdUserAkhir; //untuk menyimpan kode_user paling akhir, nanti ditambah 1 untuk setiap tambahan user baru
const char AdminUsr[]="admin"; //user admin
const char AdminPsw[]="owner777"; //password admin
int LogLev; //1=admin atau 2=users
char LogNama[26]; //untuk menyimpan nama_user yang sedang login
char LogKdUser[5]; //untuk menyimpan kode_user yang sedang login
int menu; //untuk pilihan menu utama
// <<< Variabel/Konstanta (Global)

// Prototype fungsi
void SpasiKanan(char *c);
void LoadFileDataUserKeArray(User *dt);
void LoadFileDataHistoryKeArray(History *dt);
void EditNamaUser(User *dt1);

char* BMI(float br,int tg,int i) { //Body Mass Index
    float bmi, tg2;
    static char ret[300]; memset(ret,0,sizeof(ret));
    tg2=(float)tg/100; ///konversi centimeter ke meter
    bmi=br/(tg2*tg2); //rumus BMI
    if(i==1) { //untuk tampilan informasi detail
        if (bmi<18.5) {
            strcpy(ret,"  KATEGORI: Kurus (Underweight)\n");
            strcat(ret,"  ANJURAN:\n");
            strcat(ret,"  - Makanan: Tambahkan makanan tinggi kalori seperti alpukat, kacang-kacangan, dan produk susu penuh lemak\n");
            strcat(ret,"  - Minuman: Susu protein tinggi atau smoothies dengan buah-buahan dan yogurt\n");
            strcat(ret,"  - Olahraga: Latihan kekuatan seperti angkat beban untuk membangun otot");
        }
        else if (bmi<25) {
            strcpy(ret,"  KATEGORI: Normal/Ideal (Healthy Weight)\n");
            strcat(ret,"  ANJURAN:\n");
            strcat(ret,"  - Makanan: Pertahankan pola makan seimbang dengan sayuran, buah-buahan, protein tanpa lemak, dan biji-bijian\n");
            strcat(ret,"  - Minuman: Air putih atau teh hijau tanpa gula\n");
            strcat(ret,"  - Olahraga: Kombinasi antara latihan kardio (lari, bersepeda) dan kekuatan");
        } else if (bmi<30) {
            strcpy(ret,"  KATEGORI: Berat berlebih (Overweight)\n");
            strcat(ret,"  ANJURAN:\n");
            strcat(ret,"  - Makanan: Pilih makanan rendah kalori seperti sayuran hijau, protein tanpa lemak, dan biji-bijian utuh\n");
            strcat(ret,"  - Minuman: Air putih atau infused water; hindari minuman manis\n");
            strcat(ret,"  - Olahraga: Latihan kardio intensitas sedang seperti jalan cepat atau aerobik");
        } else {
            strcpy(ret,"  KATEGORI: Obesitas (Obesity)\n");
            strcat(ret,"  ANJURAN:\n");
            strcat(ret,"  - Makanan: Fokus pada makanan rendah kalori dengan porsi terkontrol seperti sayuran, protein tanpa lemak, dan kacang-kacangan\n");
            strcat(ret,"  - Minuman: Air putih, hindari minuman berkalori tinggi seperti soda atau jus kemasan\n");
            strcat(ret,"  - Olahraga: Mulai dengan olahraga ringan seperti jalan kaki atau berenang, kemudian tingkatkan intensitas secara bertahap");
        }
    } else if(i==2) { //untuk tampilan informasi pendek
        if (bmi<18.5) strcpy(ret,"Kurus");
        else if (bmi>=18.5 && bmi<25) strcpy(ret,"Normal/Ideal");
        else if (bmi>=25 && bmi<30) strcpy(ret,"Berat berlebih");
        else strcpy(ret,"Obesitas");
    }
    return ret;
}

char* BMR(int jk,int umr,float br,int tg,char* kd) {  //Basal Metabolic Rate (berdasarkan RUMUS: MIFFLIN-ST JEOR)
    float kh;
    static char ret[7];
    kh = 10 * br + 6.25 * tg - 5 * umr + 5;  ///testing sementara
    if(strcmp(kd,"101")==0) kh *= 1.2; //sedentari
    else if(strcmp(kd,"102")==0) kh *= 1.375; //ringan
    else if(strcmp(kd,"103")==0) kh *= 1.55; //sedang
    else kh *= 1.725; //berat
    sprintf(ret,"%7.2f",kh); ///konversi float ke array char
    return ret;
}

int PeriksaFileData(char fldt[10]) { 
    FILE *fdt; 
    printf(_BB);
    if((fdt=fopen(fldt,"r"))==NULL) { 
        fdt=fopen(fldt,"w"); 
        fclose(fdt); 
        printf("    File Data [%s] Tidak Ditemukan!  File data baru dibuat..\n",fldt); printf(_N);
        return 0;
    } else {
        int c=0;
        char ch;
        while((ch=fgetc(fdt))!=EOF) if(ch=='\n') c++; 
        fclose(fdt); 
        printf("    File Data [%s] Ditemukan.  Jumlah data: %i\n",fldt,c); printf(_N);
        return c;
    }
}

void LoadFileDataUserKeArray(User *dt) { 
    FILE *fdt; 
    fdt=fopen(FlUser,"r"); 
    if (fdt == NULL) return;
    char baris[100], c[1]; 
    int ke=0; 
    while(fgets(baris,sizeof(baris),fdt)) {  
        for(int i=0; i<25; i++) dt[ke].nama[i]=baris[i]; dt[ke].nama[25]='\0'; 
        c[0]=baris[26]; dt[ke].jkel=atoi(c); 
        for(int i=28; i<36; i++) dt[ke].usr[i-28]=baris[i]; dt[ke].usr[8]='\0'; 
        for(int i=37; i<45; i++) dt[ke].psw[i-37]=baris[i]; dt[ke].psw[8]='\0'; 
        for(int i=46; i<50; i++) dt[ke].kdusr[i-46]=baris[i]; dt[ke].kdusr[4]='\0'; 
        KdUserAkhir=atoi(dt[ke].kdusr);
        ke++;
    }
    printf(_BB); printf("    Load File Data [%s] berhasil.\n",FlUser); printf(_N);
    fclose(fdt);
}

void LoadFileDataHistoryKeArray(History *dt) { 
    FILE *fdt; 
    fdt=fopen(FlHistory,"r");  
    if (fdt == NULL) return;
    char baris[100], c[5]; 
    int ke=0; 
    while(fgets(baris,sizeof(baris),fdt)) {  
        memset(c,0,sizeof(c)); for(int i=0; i<2; i++) c[i]=baris[i]; dt[ke].umur=atoi(c); 
        memset(c,0,sizeof(c)); for(int i=3; i<8; i++) c[i-3]=baris[i]; dt[ke].berat=atof(c); 
        memset(c,0,sizeof(c)); for(int i=9; i<12; i++) c[i-9]=baris[i]; dt[ke].tinggi=atoi(c); 
        dt[ke].kdaktiv[0]=baris[13]; dt[ke].kdaktiv[1]='\0'; 
        for(int i=15; i<23; i++) dt[ke].tgl[i-15]=baris[i]; dt[ke].tgl[8]='\0'; 
        for(int i=24; i<28; i++) dt[ke].kdusr[i-24]=baris[i]; dt[ke].kdusr[4]='\0'; 
        ke++;
    }
    printf(_BB); printf("    Load File Data [%s] berhasil.\n",FlHistory); printf(_N);
    fclose(fdt);
}

void DaftarAkunBaru(User *dt1) { 
    if(BufferDtUser>0) {
        User user; 
        FILE *fdt; 
        fdt=fopen(FlUser,"a"); 
        clrscr(); printf("\n\n");
        printf(_BB); printf("  DAFTAR AKUN BARU\n"); printf(_N);
        printf(_NG); printf("  (Sisa Buffer: %i)\n",BufferDtUser); printf(_N); 
        printf("  -----------------------------------\n"); getchar();
        printf("  N a m a  L e n g k a p             : "); scanf("%[^\n]",user.nama); 
        printf("  Jenis kelamin (1 Laki, 2 Perempuan): "); scanf(" %d",&user.jkel);
        printf("  Nama Akun (max 8, tanpa spasi)     : "); scanf("%8s",user.usr);
        printf("  Password (max 8, tanpa spasi)      : "); scanf("%8s",user.psw);
        KdUserAkhir++;
        sprintf(user.kdusr,"%d",KdUserAkhir); 
        fprintf(fdt,"%-25s %-1d %-8s %-8s %-4s\n",user.nama,user.jkel,user.usr,user.psw,user.kdusr);  
        LoadFileDataUserKeArray(dt1);  
        BufferDtUser--; JmlDtUser++; 
        fclose(fdt);
    } else {
        printf(_NR); printf("\n\n  Maaf.. Buffer Data telah habis !\n"); printf(_N); 
        printf("  Untuk Reset Buffer, Silahkan Keluar Program lalu masuk kembali\n");
        printf("  Tekan [Enter] untuk kembali ke Menu"); getchar(); getchar();
    }
}

void TambahHistory(History *dt1,Aktivitas *dt2) { 
    if(BufferDtHistory>0) {
        History hstr; 
        FILE *fdt; 
        fdt=fopen(FlHistory,"a"); 
        int pilaktiv;
        clrscr(); printf("\n\n");
        printf(_BB); printf("  TAMBAH HISTORY\n"); printf(_N);
        printf(_NG); printf("  (Sisa Buffer: %i)\n",BufferDtHistory); printf(_N); 
        printf("  -----------------------------------\n");
        printf(_BY); printf("  %s",LogNama); printf(_N); 
        printf(_NY); printf(" [%s]\n\n",LogKdUser); printf(_N); 
        getchar(); 
        printf("  Umur (dalam tahun)      : "); scanf("%i",&hstr.umur);
        printf("  Berat Badan (dalam kg)  : "); scanf("%f",&hstr.berat);
        printf("  Tinggi badan (dalam cm) : "); scanf("%i",&hstr.tinggi);
        printf("  Tingkat Aktivitas (BMR) :\n");
        for(int i=0;i<4;i++) printf("    %d  %s\n",i+1,dt2[i].aktivitas);  
        printf("    Pilihan: "); scanf("%i",&pilaktiv);  
        strcpy(hstr.kdaktiv,dt2[pilaktiv-1].kdaktiv);  
        printf("  Tanggal (DDMMYYYY) : "); scanf("%s",hstr.tgl);
        fprintf(fdt,"%-2i %-5.1f %-3i %-1s %-8s %-4s\n",hstr.umur,hstr.berat,hstr.tinggi,hstr.kdaktiv,hstr.tgl,LogKdUser);  
        LoadFileDataHistoryKeArray(dt1);  
        BufferDtHistory--; JmlDtHistory++;  
        fclose(fdt);
    } else {
        printf(_NR); printf("\n\n  Maaf.. Buffer Data telah habis !\n"); printf(_N); 
        printf("  Untuk Reset Buffer, Silahkan Keluar Program lalu masuk kembali\n");
        printf("  Tekan [Enter] untuk kembali ke Menu"); getchar(); getchar();
    }
}

void SpasiKanan(char *c) { 
    int len = strlen(c);
    for(int i = len - 1; i >= 0; i--) {
        if(c[i] == ' ') c[i] = '\0';
        else break;
    }
}

void EditNamaUser(User *dt1) {
    char perbaikan, nama[26];
    clrscr(); printf("\n\n");
    printf(_BB); printf("  EDIT/UPDATE NAMA USER\n");
    printf(_N);  printf("  ----------------------\n"); 
    printf(_N); printf(_BG); printf("  %s  [%s]\n\n",LogNama,LogKdUser); printf(_N);
    getchar(); 
    printf("  Ketik Perbaikan Nama : "); printf(_BG); scanf("%[^\n]",nama); printf(_N);
    do{
        getchar(); printf("\n\n  Simpan Perbaikan (Y/T) ? "); scanf("%c",&perbaikan); 
        if(toupper(perbaikan)!='Y' && toupper(perbaikan)!='T') {printf("\n  Simpan Perbaikan (Y/T) ? ");}
    } while(toupper(perbaikan)!='Y' && toupper(perbaikan)!='T');
    if(toupper(perbaikan)=='Y') {
        strcpy(LogNama,nama); 
        char baris[100];
        FILE *fdt1, *fdt2; 
        fdt1=fopen(FlUser,"r"); 
        fdt2=fopen(FlUser2,"w"); 
        while((fgets(baris,sizeof(baris),fdt1))!=NULL) fputs(baris,fdt2); 
        fclose(fdt1); fclose(fdt2); 
        fdt1=fopen(FlUser,"w"); 
        for(int i=0;i<JmlDtUser;i++) { 
            if(strcmp(dt1[i].kdusr,LogKdUser)!=0) fprintf(fdt1,"%-25s %-1d %-8s %-8s %-4s\n",dt1[i].nama,dt1[i].jkel,dt1[i].usr,dt1[i].psw,dt1[i].kdusr);  
            else fprintf(fdt1,"%-25s %-1d %-8s %-8s %-4s\n",LogNama,dt1[i].jkel,dt1[i].usr,dt1[i].psw,dt1[i].kdusr);  
        }
        fclose(fdt1);
        LoadFileDataUserKeArray(dt1);  
    }
}

void LihatDaftarUser(User *dt1,History *dt2,Aktivitas *dt3) { 
    char nm[25];
    int hstr,activ;
    clrscr(); printf("\n\n");
    printf(_BY); printf("                                            D A F T A R   P E N G G U N A\n"); 
    printf(_N);
    printf("  =======================================================================================================================\n");
    printf(_NY);
    printf("  N a m a                     Jenis       Umur  Berat  Tinggi   Tingkat Aktivitas                    B M I             B M R\n");
    printf("  P e n g g u n a             Kelamin                                                                           Kal/hari\n");
    printf(_N);
    printf("  =======================================================================================================================\n");
    for(int i=0;i<JmlDtUser;i++) { 
        strcpy(nm,dt1[i].nama); SpasiKanan(nm); strcat(nm," ["); strcat(nm,dt1[i].kdusr); strcat(nm,"]");  
        hstr=-1; activ=-1; 
        for(int j=0;j<JmlDtHistory;j++) { 
            if(strcmp(dt2[j].kdusr,dt1[i].kdusr)==0) hstr=j;
        }
        for(int j=0;j<4;j++) { 
            if(strcmp(dt2[i].kdaktiv,dt3[j].kdaktiv)==0) activ=j;
        }
        if(hstr<0) {
            printf(_BG); printf("  %-25s %s",nm,(dt1[i].jkel==1?"Laki-laki":"Perempuan")); printf(_N); 
            printf(_IY); printf("  -- data histories belum ada --\n"); printf(_N);
        } else {
            printf(_BG); 
            printf("  %-25s %s  %i    %5.1f     %3i   %s   %-15s  %s\n",nm,(dt1[i].jkel==1?"Laki-laki":"Perempuan"),dt2[hstr].umur,dt2[hstr].berat,dt2[hstr].tinggi,dt3[activ].aktivitas,BMI(dt2[hstr].berat,dt2[hstr].tinggi,2),BMR(dt1[i].jkel,dt2[hstr].umur,dt2[hstr].berat,dt2[hstr].tinggi,dt3[activ].kdaktiv)); 
            printf(_N);
        }
    }
    printf(_N);
    printf("  -----------------------------------------------------------------------------------------------------------------------\n");
    printf(_NY); 
    printf("  Jumlah Data: %d\n\n",JmlDtUser); 
    printf(_N); printf(_BB); 
    printf("  Keterangan:\n"); 
    printf("  B M I (Body Mass Index)  adalah Ukuran Lemak Tubuh\n"); 
    printf("  B M R (Basal Metabolic Rate) Menggunakan Rumus: MIFFLIN-ST JEOR  adalah Kebutuhan Kalori PerHari\n"); 
    printf(_N); printf(_IY);
    printf("  Catatan: Umur, Berat, Tinggi dan Tingkat Aktivitas adalah berdasar data histories inputan terakhir\n"); 
    printf(_N);
    printf("  =======================================================================================================================");
    printf("\n\n  Tekan [Enter] untuk kembali ke Menu"); getchar(); getchar();
}

void HapusHistory(History *dt2,Aktivitas *dt3) {
    int nomor,pilnomor,activ;
    char tgl[10];
    clrscr(); printf("\n\n");
    printf(_BB); printf("  HAPUS HISTORY USER\n");
    printf(_N);  printf("  ------------------\n"); 
    printf(_N); printf(_BG); printf("  %s  [%s]\n\n",LogNama,LogKdUser); printf(_N);
    printf(_NY); printf("  Data Histories:\n"); printf(_N);
    nomor=0; activ=-1; 
    printf("  ------------------------------------------------------------------------\n");
    printf(_NY); printf("  Nomor Umur  Berat  Tinggi   Tingkat Aktivitas                  Tanggal   \n"); printf(_N);
    printf("  ------------------------------------------------------------------------\n");
    printf(_BG);
    for(int j=0;j<JmlDtHistory;j++) { 
        if(strcmp(dt2[j].kdusr,LogKdUser)==0) {
            for(int k=0;k<4;k++) if(strcmp(dt2[j].kdaktiv,dt3[k].kdaktiv)==0) activ=k;
            nomor++;
            sprintf(tgl,"%.2s-%.2s-%.4s",dt2[j].tgl,dt2[j].tgl+2,dt2[j].tgl+4); 
            printf("  %4i  %4i  %5.1f     %3i   %32s  %s\n",nomor,dt2[j].umur,dt2[j].berat,dt2[j].tinggi,dt3[activ].aktivitas,tgl);
        }
    }
    if(nomor==0) { printf(_IY); printf("  data histories belum ada\n"); }
    printf(_N); printf("  ------------------------------------------------------------------------\n\n");
    printf("  Pilih Nomor History yang ingin diHapus\n  atau ketik 0 untuk Batal ? "); scanf("%i",&pilnomor);
    if(pilnomor!=0 && nomor!=0) {
        char baris[100];
        FILE *fdt1, *fdt2; 
        fdt1=fopen(FlHistory,"r"); 
        fdt2=fopen(FlHistory2,"w"); 
        while((fgets(baris,sizeof(baris),fdt1))!=NULL) fputs(baris,fdt2); 
        fclose(fdt1); fclose(fdt2); 
        fdt1=fopen(FlHistory,"w"); 
        for(int i=0;i<JmlDtHistory;i++) { 
            if(i!=pilnomor-1) {
                fprintf(fdt1,"%-2i %-5.1f %-3i %-1s %-8s %-4s\n",dt2[i].umur,dt2[i].berat,dt2[i].tinggi,dt2[i].kdaktiv,dt2[i].tgl,dt2[i].kdusr);   
            }
        }
        fclose(fdt1);
        LoadFileDataHistoryKeArray(dt2);  
        BufferDtHistory++; JmlDtHistory--;  
    } 
}

void HapusAkunUser(User *dt1,History *dt2) {
    bool ketemu;
    char kdusr[5];
    char hapus;
    clrscr(); printf("\n\n");
    printf(_BB); printf("  HAPUS AKUN USER\n");
    printf(_N);  printf("  ---------------\n"); 
    printf("  Kode User (4 angka): "); printf(_BG); scanf("%4s",kdusr);
    ketemu=false;
    for(int i=0;i<JmlDtUser;i++) { 
        if(strcmp(dt1[i].kdusr,kdusr)==0) { 
            ketemu=true;
            printf(_N); printf("  Nama Pengguna      :"); printf(_BG); printf(" %s\n",dt1[i].nama);
            printf(_N); printf("  Jenis Kelamin      :"); printf(_BG); printf(" %s\n\n",(dt1[i].jkel==1?"Laki-laki":"Perempuan"));
            break; 
        }
    }
    if(!ketemu) {
        printf(_NR); printf("\n  Kode User Tidak Ditemukan !\n"); printf(_N);
        printf("\n  Tekan [Enter] untuk kembali ke Menu"); getchar(); getchar();     
    } else {
        printf(_NR); printf("\n  Proses Hapus Akun akan juga menghapus semua data Histories akun ini !\n"); printf(_N);
        do{
            getchar(); printf("\n  Lanjut Proses Hapus (Y/T) ? "); scanf("%c",&hapus); 
            if(toupper(hapus)!='Y' && toupper(hapus)!='T') {printf("\n  Lanjut Proses Hapus (Y/T) ? ");}
        } while(toupper(hapus)!='Y' && toupper(hapus)!='T');
        if(toupper(hapus)=='Y') {
            char baris[100];
            FILE *fdt1, *fdt2; 
            fdt1=fopen(FlUser,"r"); 
            fdt2=fopen(FlUser2,"w"); 
            while((fgets(baris,sizeof(baris),fdt1))!=NULL) fputs(baris,fdt2); 
            fclose(fdt1); fclose(fdt2); 
            fdt1=fopen(FlHistory,"r"); 
            fdt2=fopen(FlHistory2,"w"); 
            while((fgets(baris,sizeof(baris),fdt1))!=NULL) fputs(baris,fdt2); 
            fclose(fdt1); fclose(fdt2); 
            fdt1=fopen(FlUser,"w"); 
            for(int i=0;i<JmlDtUser;i++) { 
                if(strcmp(dt1[i].kdusr,kdusr)!=0) fprintf(fdt1,"%-25s %-1d %-8s %-8s %-4s\n",dt1[i].nama,dt1[i].jkel,dt1[i].usr,dt1[i].psw,dt1[i].kdusr);  
            }
            fclose(fdt1);
            fdt2=fopen(FlHistory,"w"); 
            for(int i=0;i<JmlDtHistory;i++) { 
                if(strcmp(dt2[i].kdusr,kdusr)!=0) fprintf(fdt2,"%-2i %-5.1f %-3i %-1s %-8s %-4s\n",dt2[i].umur,dt2[i].berat,dt2[i].tinggi,dt2[i].kdaktiv,dt2[i].tgl,dt2[i].kdusr);   
            }
            fclose(fdt2);
            LoadFileDataUserKeArray(dt1);  
            BufferDtUser++; JmlDtUser--; 
            LoadFileDataHistoryKeArray(dt2);  
            BufferDtHistory++; JmlDtHistory--; 
        }
    }
}

void LihatDetailUser(User *dt1,History *dt2,Aktivitas *dt3,int i) { 
    bool ketemu;
    int nomor,activ,hstr;
    char kdusr[5], tgl[10];
    char ulang;    
    ulang='Y';
    do{
        clrscr(); printf("\n\n");
        printf(_BB); printf("  LIHAT DETAIL USER\n");
        printf(_N);  printf("  -----------------\n"); 
        if(i==1) { printf("  Kode User (4 angka): "); printf(_BG); scanf("%4s",kdusr); }
        else strcpy(kdusr,LogKdUser);
        ketemu=false; nomor=0; hstr=-1; 
        for(int idx=0;idx<JmlDtUser;idx++) { 
            if(strcmp(dt1[idx].kdusr,kdusr)==0) { 
                ketemu=true;
                printf(_N); printf("  Nama Pengguna      :"); printf(_BG); printf(" %s\n",dt1[idx].nama);
                printf(_N); printf("  Jenis Kelamin      :"); printf(_BG); printf(" %s\n\n",(dt1[idx].jkel==1?"Laki-laki":"Perempuan"));
                printf(_NY); printf("  Data Histories:\n"); printf(_N);
                activ=-1; 
                printf("  ------------------------------------------------------------------------\n");
                printf(_NY); printf("  Nomor Umur  Berat  Tinggi   Tingkat Aktivitas                  Tanggal   \n"); printf(_N);
                printf("  ------------------------------------------------------------------------\n");
                printf(_BG);
                for(int j=0;j<JmlDtHistory;j++) { 
                    if(strcmp(dt2[j].kdusr,dt1[idx].kdusr)==0) {
                        hstr=j; 
                        for(int k=0;k<4;k++) if(strcmp(dt2[j].kdaktiv,dt3[k].kdaktiv)==0) activ=k;
                        nomor++;
                        sprintf(tgl,"%.2s-%.2s-%.4s",dt2[j].tgl,dt2[j].tgl+2,dt2[j].tgl+4); 
                        printf("  %4i  %4i  %5.1f     %3i   %32s  %s\n",nomor,dt2[j].umur,dt2[j].berat,dt2[j].tinggi,dt3[activ].aktivitas,tgl);
                    }
                }
                if(nomor==0) {printf(_IY); printf("  data histories belum ada\n");}
                printf(_N); printf("  ------------------------------------------------------------------------\n\n");
                if(nomor>0) {
                    printf(_BY); printf("  Berdasar data Histories terakhir:\n"); printf(_N);
                    printf("\n  Hasil BMI dan Anjuran :\n"); printf(_BG); printf("%s\n",BMI(dt2[hstr].berat,dt2[hstr].tinggi,1)); printf(_N);
                    printf("\n  Hasil BMR :\n"); printf(_BG);
                    printf("  Anda membutuhkan: %s Kalori/Hari\n",BMR(dt1[idx].jkel,dt2[hstr].umur,dt2[hstr].berat,dt2[hstr].tinggi,dt3[activ].kdaktiv)); 
                    printf("  Silahkan pilih jenis dan jumlah makanan sesuai ukuran kalori/gram nya"); printf(_N);
                }
                break; 
            } 
        }
        if(i==1) { 
            if(!ketemu) { printf(_NR); printf("\n  Kode User Tidak Ditemukan !\n"); printf(_N);}
            do{
                printf("\n\n  Cari lagi (Y/T)? "); scanf(" %c",&ulang); 
                if(toupper(ulang)!='Y' && toupper(ulang)!='T') {printf("\n\n  Cari lagi (Y/T)? ");}
            } while(toupper(ulang)!='Y' && toupper(ulang)!='T');
        } else { printf("\n\n  Tekan [Enter] untuk kembali ke Menu"); getchar(); getchar(); ulang='T'; }
    } while(toupper(ulang)=='Y');
}

void MenuAdmin(User *dt1,History *dt2,Aktivitas *dt3) { 
    clrscr(); printf("\n\n"); 
    printf(_NG); printf("\n  Loading File Data ke Memory..\n"); printf(_N);
    LoadFileDataUserKeArray(dt1); 
    LoadFileDataHistoryKeArray(dt2); 
    my_sleep(2);
    int pilih;
    do {
        clrscr(); printf("\n\n"); 
        printf(_BY);
        printf("  ==================================================\n");
        printf("             A   P   L   I   K   A   S   I          \n");
        printf("  BASAL METABOLIC RATE (BMR) & BODY MASS INDEX (BMI)\n");
        printf("  ==================================================\n\n");
        printf(_N); printf(_BB); printf("  Logon by");
        printf(_N); printf(_BG); printf(" %s\n\n",LogNama); printf(_N);
        printf("                M E N U   A D M I N\n"); 
        printf("                ------------------\n\n");
        printf(_N); printf(_BG);
        printf("                1  Lihat Daftar Users\n");
        printf("                2  Lihat Detail User\n");
        printf("                3  Hapus Akun User + Histories\n");
        printf("                4  Exit\n");
        printf(_BB); printf("\n");
        printf("                Pilihan: "); scanf("%i",&pilih);
        printf(_N);
        switch (pilih) {
            case 1: LihatDaftarUser(dt1,dt2,dt3); break;
            case 2: LihatDetailUser(dt1,dt2,dt3,1); break;
            case 3: HapusAkunUser(dt1,dt2); break;
            case 4: menu=3; break;
            default: break;
        }
    } while (pilih!=4);
}

void MenuUser(User *dt1,History *dt2,Aktivitas *dt3) { 
    clrscr(); printf("\n\n"); 
    printf(_NG); printf("\n  Loading File Data ke Memory..\n"); printf(_N);
    LoadFileDataUserKeArray(dt1); 
    LoadFileDataHistoryKeArray(dt2); 
    my_sleep(2);
    int pilih;
    do {
        clrscr(); printf("\n\n"); 
        printf(_BY);
        printf("  ==================================================\n");
        printf("             A   P   L   I   K   A   S   I          \n");
        printf("  BASAL METABOLIC RATE (BMR) & BODY MASS INDEX (BMI)\n");
        printf("  ==================================================\n\n");
        printf(_N); printf(_BB); printf("  Logon by");
        printf(_N); printf(_BG); printf(" %s  [%s]\n\n",LogNama, LogKdUser);
        printf(_N); printf(_BG);
        printf("                1  Lihat Detail User & History\n");
        printf("                2  Tambah Data History\n");
        printf("                3  Edit/Update Nama User\n");
        printf("                4  Hapus Data History\n");
        printf("                5  Exit\n");
        printf(_BB); printf("\n");
        printf("                Pilihan: "); scanf("%i",&pilih);
        printf(_N);
        switch (pilih) {
            case 1: LihatDetailUser(dt1,dt2,dt3,2); break;
            case 2: TambahHistory(dt2,dt3); break;
            case 3: EditNamaUser(dt1); break;
            case 4: HapusHistory(dt2,dt3); break;
            case 5: menu=3; break;
            default: break;
        }
    } while (pilih!=4);
}

int main() {
    User *dtUser = malloc(sizeof(User) * 100);
    History *dtHistory = malloc(sizeof(History) * 200);
    Aktivitas dtAktivitas[4] = {
        {"1. Sedentari (jarang olahraga)", "101"},
        {"2. Ringan (olahraga 1-3x seminggu)", "102"},
        {"3. Sedang (olahraga 3-5x seminggu)", "103"},
        {"4. Berat (olahraga 6-7x seminggu)", "104"}
    };

    JmlDtUser = PeriksaFileData(FlUser);
    JmlDtHistory = PeriksaFileData(FlHistory);
    
    if (JmlDtUser > 0) LoadFileDataUserKeArray(dtUser);
    if (JmlDtHistory > 0) LoadFileDataHistoryKeArray(dtHistory);

    menu = 0;
    do {
        clrscr();
        printf("\n\n");
        printf(_BY);
        printf("  ==================================================\n");
        printf("             A   P   L   I   K   A   S   I          \n");
        printf("  BASAL METABOLIC RATE (BMR) & BODY MASS INDEX (BMI)\n");
        printf("  ==================================================\n\n");
        printf(_N);
        printf("                U T A M A\n");
        printf("                ---------\n\n");
        printf(_BG);
        printf("                1  Login Admin\n");
        printf("                2  Login User\n");
        printf("                3  Daftar Akun Baru\n");
        printf("                4  Keluar Program\n");
        printf(_BB); printf("\n");
        printf("                Pilihan: "); scanf("%i",&menu);
        printf(_N);

        if (menu == 1) {
            char usr[20], psw[20];
            clrscr();
            printf("\n\n  LOGIN ADMIN\n  -----------\n");
            printf("  Username: "); scanf("%s", usr);
            printf("  Password: "); scanf("%s", psw);
            if (strcmp(usr, AdminUsr) == 0 && strcmp(psw, AdminPsw) == 0) {
                LogLev = 1;
                strcpy(LogNama, "Administrator");
                MenuAdmin(dtUser, dtHistory, dtAktivitas);
            } else {
                printf(_NR); printf("\n  Username atau Password Admin Salah!\n"); printf(_N);
                printf("  Tekan [Enter] untuk lanjut..."); getchar(); getchar();
            }
        } 
        else if (menu == 2) {
            char usr[20], psw[20];
            clrscr();
            printf("\n\n  LOGIN USER\n  ----------\n");
            printf("  Username: "); scanf("%s", usr);
            printf("  Password: "); scanf("%s", psw);
            
            LoadFileDataUserKeArray(dtUser);
            bool found = false;
            for (int i = 0; i < JmlDtUser; i++) {
                if (strcmp(dtUser[i].usr, usr) == 0 && strcmp(dtUser[i].psw, psw) == 0) {
                    found = true;
                    LogLev = 2;
                    strcpy(LogNama, dtUser[i].nama);
                    strcpy(LogKdUser, dtUser[i].kdusr);
                    MenuUser(dtUser, dtHistory, dtAktivitas);
                    break;
                }
            }
            if (!found) {
                printf(_NR); printf("\n  Username atau Password User Salah!\n"); printf(_N);
                printf("  Tekan [Enter] untuk lanjut..."); getchar(); getchar();
            }
        } 
        else if (menu == 3) {
            LoadFileDataUserKeArray(dtUser);
            DaftarAkunBaru(dtUser);
        }
    } while (menu != 4);

    free(dtUser);
    free(dtHistory);
    return 0;
}

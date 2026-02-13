#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define _N "\033[0m" //Normal
#define _NG "\033[32m" //normal-green
#define _NY "\033[33m" //normal-yellow
#define _NR "\033[31m" //normal-red
#define _BG "\033[1;32m" //bold-green
#define _BY "\033[1;33m" //bold-yellow
#define _BB "\033[1;34m" //bold-blue
#define _BGG "\033[42m" // Latar Belakang Hijau 
#define _BM "\033[1;35m" // Tebal Magenta
#define _BC "\033[1;36m" // Tebal Cyan   

#ifdef _WIN32
#define clrscr() system("cls")
#else
#define clrscr() system("clear")
#endif

typedef struct {
    char nama[26];
    char gender;
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
} Data;
typedef struct {
    char aktivitas[35];
    char kdaktiv[4];
} Aktivitas;

char* FlUser="users.txt";
char* FlHistory="histories.txt";  //nama file data untuk history

int JmlDtUser;
int BufferDtUser=50;
int KdUserAkhir;


const char AdminUsr[]="admin"; //user admin
const char AdminPsw[]="euforia"; //password admin
int LogLev; //1=admin atau 2=users
char LogNama[26]; //untuk menyimpan nama_user yang sedang login
char LogKdUser[5]; //untuk menyimpan kode_user yang sedang login

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//File
int PeriksaFileData(char fldt[10]) { ///param: nama_file di var global: FlUser atau FlHistory
    FILE *fdt; ///deklarasi variabel tipe file
    if((fdt=fopen(fldt,"r"))==NULL) { ///coba buka file data, bila NULL artinya file data belum ada (r = untuk dibaca saja)
        fdt=fopen(fldt,"w"); ///buat file data baru (w = buat baru & untuk diisi)
        fclose(fdt); ///tutup file data
        printf(_BB "  File Data [%s] Tidak Ditemukan!  File data baru dibuat..\n" _N,fldt);
        return 0;
    } else {
        int c=0;
        char ch;
        while((ch=fgetc(fdt))!=EOF) if(ch=='\n') c++; ///baca file data baris-perbaris (untuk hitung jumlah data)
        fclose(fdt); ///tutup file data
        printf(_BB "  File Data [%s] Ditemukan.  Jumlah data: %i\n" _N,fldt,c);
        BufferDtUser=50-c;
        KdUserAkhir=1000+c;
        JmlDtUser=c;
    }
}
void LoadFileDataUserKeArray(User *dt) {

    FILE *fdt;
    fdt=fopen(FlUser,"r");
    char baris[100], c[1];
    int ke=0;
    while(fgets(baris,sizeof(baris),fdt)) { 

        for(int i=0; i<25; i++) dt[ke].nama[i]=baris[i]; dt[ke].nama[25]='\0';
        dt[ke].gender = toupper(baris[26]); //c[0]=baris[26]; dt[ke].gender=atoi(c);
        for(int i=28; i<36; i++) dt[ke].usr[i-28]=baris[i]; dt[ke].usr[8]='\0';
        for(int i=37; i<45; i++) dt[ke].psw[i-37]=baris[i]; dt[ke].psw[8]='\0';
        for(int i=46; i<50; i++) dt[ke].kdusr[i-46]=baris[i]; dt[ke].kdusr[4]='\0';
        KdUserAkhir=atoi(dt[ke].kdusr);
        ke++;
    }
    {printf(_BB); printf("  Load File Data [%s] berhasil.\n",FlUser); printf(_N);}
    fclose(fdt);
}
//File

//Untuk daftar User
void menudaftar() {
    clrscr();
    printf(_BB "  DAFTAR AKUN BARU\n"); printf(_N);
    printf(_NG "  (Sisa Buffer: %i)\n",BufferDtUser);
    printf(_N "  -----------------------------------------------\n");
}

void inputNamaLengkap(const char *keterangan, char *input, int maks) {
    while (1) {
        menudaftar();
        printf("%s (Max %d karakter): ", keterangan, maks);
        fgets(input, maks + 2, stdin) == NULL;

        // Hapus karakter newline jika ada
        input[strcspn(input, "\n")] = '\0';

        // Validasi panjang input
        if (strlen(input) > maks) {
            printf(_NR "  Input terlalu panjang. Coba lagi.\n" _N);
            usleep(800000);
            clear_input_buffer(); // Bersihkan buffer jika input terlalu panjang
        } else if (strlen(input) == 0) {
            printf(_NR "  Input tidak boleh kosong. Coba lagi.\n" _N);
            usleep(800000);
        } else {
            // Input valid
            break;
        }
    }
}
void inputGender(User *user) {
    while (1) {
        clrscr();
        menudaftar();
        printf(_N"  Gender (L/P): ");
        char gender_input[3];
        fgets(gender_input, sizeof(gender_input), stdin);
        gender_input[strcspn(gender_input, "\n")] = '\0';

        if (strcasecmp(gender_input, "L") == 0) {
            user->gender = 'L';
            break;
        } else if (strcasecmp(gender_input, "P") == 0) {
            user->gender = 'P';
            break;
        } else {
            clear_input_buffer();
        }
    } 
}
void inputUserPass(const char *keterangan, char *input, int maks) {
    while (1) {
        menudaftar();
        printf(_N "%s (Max %d karakter, tanpa spasi): ", keterangan, maks);
        fgets(input, maks + 2, stdin); // +2 untuk NUll karakter
        input[strcspn(input, "\n")] = '\0'; // Bersihkan karakter \n

        if (strlen(input) > maks) {
            printf(_NR "  Input terlalu panjang. Coba lagi.\n"); usleep(500000);
            clear_input_buffer();
        } else if (strchr(input, ' ') != NULL) {
            printf(_NR"  Input tidak boleh mengandung spasi. Tekan Enter\n" _N);
            clear_input_buffer();
        } else {
            break;
        }
    }
}

void namaLengkap(User *user) {
    inputNamaLengkap("  Nama Lengkap", user->nama, 25);
}
void username(User *user) {
    inputUserPass("  Nama Akun", user->usr, 8);
}
void password(User *user) {
    inputUserPass("  Password", user->psw, 8);
}

void DaftarAkunBaru(User *dt1) {
    if(BufferDtUser>0) {
        User user;
        FILE *fdt;
        fdt=fopen(FlUser,"a");
        clrscr();
        namaLengkap(&user);
        inputGender(&user);
        username(&user);
        password(&user);
        clear_input_buffer;
        char batal;
        while (toupper(batal)!='Y' && toupper(batal)!='T') {
            menudaftar();
            printf("  Nama Lengkap : %s\n",user.nama);
            printf("  Gender       : %c\n",user.gender);
            printf("  Username     : %s\n",user.usr);
            printf("  Password     : %s\n\n",user.psw);
            printf(_NY "  Apakah data anda sudah benar? (Y/T):" _N); scanf("%c",&batal);
        } if (toupper(batal) == 'Y') {
            clrscr();
            KdUserAkhir++;
            sprintf(user.kdusr,"%d",KdUserAkhir); ///konversi int ke array char
            fprintf(fdt, "%-25s %-1c %-8s %-8s %-4s\n", user.nama, user.gender, user.usr, user.psw, user.kdusr);
            LoadFileDataUserKeArray(dt1);  
            BufferDtUser--; JmlDtUser++;
            clrscr(); 
            fclose(fdt);
            clear_input_buffer();
            printf(_NG "  Akun anda telah disimpan. Tekan Enter untuk kembali");
            getchar();
            return;
        } else if (toupper(batal) == 'T') {
            clear_input_buffer();
            printf(_NR "  Pendaftaran dibatalkan. Tekan Enter untuk kembali");
            getchar();
            return;
        }
        
    } else {
        printf(_NR); printf("\n\n  Maaf.. Buffer Data telah habis !\n"); printf(_N); 
        printf("  Tekan [Enter] untuk kembali ke Menu"); getchar(); getchar();
    }
}
//Untuk Daftar User

//Untuk Login
void menuLogin() {
    clrscr();
    printf(_BB "  L O G I N\n"); printf(_N);
    printf(_NY "  (B untuk Kembali)\n" _N);
    printf("  --------------------\n");
}
void menuAdmin(User *dt1) {
    clrscr(); printf("\n\n"); 
    printf(_NG "\n  Loading File Data ke Memory..\n" _N);
    LoadFileDataUserKeArray(dt1); //baca dari file data: user.txt lalu isi ke array: Users
    //LoadFileDataHistoryKeArray(dt2); //baca dari file data: history.txt lalu isi ke array: Histories
    sleep(1);
    int pilih;
    while (1) {
        clrscr(); printf("\n"); 
        printf(_BY " |====================================================| \n");
        printf(" |           A   P   L   I   K   A   S   I            | \n");
        printf(" |         SISTEM PENGELOLAAN DATA KESEHATAN          | \n");
        printf(_BY " |====================================================| \n" _N);
        printf(_BB "  Logon by");
        printf(_BG " %s [%s]                                            \n" ,LogNama,LogKdUser);
        printf(_BM "\n               |------------------------|                \n");
        printf(_BM "               |   "); printf(_BC "M E N U  A D M I N"); printf(_BM "   |                \n"); 
        printf(_BM "               |------------------------|                \n"_N);
        printf("       "); printf(_BGG "  "_N);   printf(_BM "      | (1) Lihat Daftar Users |      "); printf(_BGG "  "_N);      printf("    \n");
        printf("     "); printf(_BGG "      "_N); printf(_BM "    | (2) Lihat Detail User  |    ");   printf(_BGG "      "_N);  printf("    \n");
        printf("       "); printf(_BGG "  "_N);   printf(_BM "      | (3) Hapus Akun User    |      "); printf(_BGG "  "_N);      printf("    \n");
        printf(_BM "               | (4) Log Out            |                 \n"_BB);
        printf(_BM "               | (5) Exit               |                 \n"_BB);
        printf(_BM "               |------------------------|                \n");
        printf(_BB "\n                 Pilihan: " _N); scanf("%i",&pilih);
        switch (pilih) {
            case 1: return;
            case 2: return;
            case 3: return;
            case 4: clear_input_buffer(); return;
            case 5: exit(0);
            default: break;
        }
    }
}
void menuUser(User *dt1) {
    clrscr(); printf("\n\n"); 
    printf(_NG "\n  Loading File Data ke Memory..\n" _N);
    LoadFileDataUserKeArray(dt1); //baca dari file data: user.txt lalu isi ke array: Users
    //LoadFileDataHistoryKeArray(dt2); //baca dari file data: history.txt lalu isi ke array: Histories
    sleep(1);
    int pilih;
    while (1) {
        clrscr(); printf("\n"); 
        printf(_BY " |===================================================| \n");
        printf(" |           A   P   L   I   K   A   S   I           | \n");
        printf(" |         SISTEM PENGELOLAAN DATA KESEHATAN         | \n");
        printf(_BY " |===================================================| \n" _N);
        printf(_BB "  Logon by");
        printf(_BG " %s [%s]                                            \n" ,LogNama,LogKdUser);
        printf(_BM "\n               |-----------------------|                \n");
        printf(_BM "               |    "); printf(_BC "M E N U  U S E R"); printf(_BM "   |                \n"); 
        printf(_BM "               |-----------------------|                \n"_N);
        printf("       "); printf(_BGG "  "_N);   printf(_BM "      | (1) Isi/Update Data   |      "); printf(_BGG "  "_N);      printf("    \n");
        printf("     "); printf(_BGG "      "_N); printf(_BM "    | (2) Lihat Data Anda   |    ");   printf(_BGG "      "_N);  printf("    \n");
        printf("       "); printf(_BGG "  "_N);   printf(_BM "      | (3) Log Out           |      "); printf(_BGG "  "_N);      printf("    \n");
        printf(_BM "               | (4) Exit              |                 \n"_BB);
        printf(_BM "               |-----------------------|                \n");
        printf(_BB "\n                 Pilihan: " _N); scanf("%i",&pilih);
        switch (pilih) {
            case 1: return;
            case 2: return;//EditNamaUser(dt1); break;
            case 3: clear_input_buffer(); return;
            case 4: exit(0);
            default: break;
        }
    }
}

void SpasiKanan(char *c) { //untuk menghapus spasi dibagian kanan teks
    int len=strlen(c);
    while(len>0 && c[len-1]==' ') { c[len-1]='\0'; len--; }
}

void login() {
    FILE *fdt;
    fdt=fopen(FlUser,"r");
    char baris[100];
    char usr[9],psw[9], usr2[9],psw2[9];
    int ke = 0;
    LogLev=0; strcpy(LogNama,"none"); memset(LogKdUser,0,sizeof(LogKdUser)); ///reset LogLev=0, LogNama="none" dan LogKdUser=""
    menuLogin ();
    printf(_N "  Masukkan Username : ");
    scanf(" %8s", usr);
    if (strcmp(usr, "B") == 0 || strcmp(usr, "b") == 0) {
        return; // Kembali ke menu utama
    }
    clear_input_buffer();

    printf(_N "  Masukkan Password : ");
    scanf(" %8s", psw);
    if (strcmp(psw, "B") == 0 || strcmp(psw, "b") == 0) {
        return; // Kembali ke menu utama
    }
    clear_input_buffer();
    
    if (strcmp(usr, AdminUsr) == 0 && strcmp(psw, AdminPsw) == 0) {
    LogLev=1;
    strcpy(LogNama,"Admin"); 
    strcpy(LogKdUser,"0000");
    } else { ///jika login bukan Admin
        while(fgets(baris,sizeof(baris),fdt)) {  //baca 1 baris teks dari file Users.txt selama masih ada baris
            for(int i=28; i<36; i++) if(baris[i]!=' ') usr2[i-28]=baris[i]; else usr2[i-28]='\0'; ///ambil posisi karakter untuk user dari 1baris teks itu
            for(int i=37; i<45; i++) if(baris[i]!=' ') psw2[i-37]=baris[i]; else psw2[i-37]='\0'; ///ambil posisi karakter untuk password dari 1baris teks itu
            if(strcmp(usr2,usr)==0 && strcmp(psw2,psw)==0) { //jika user & password yang diiput sama dengan user & password dari file data
            LogLev=2;
            for(int i=0; i<25; i++)  LogNama[i]=baris[i]; //ambil posisi karakter untuk nama dari 1baris teks itu
            SpasiKanan(LogNama);
            for(int i=46; i<50; i++) LogKdUser[i-46]=baris[i]; LogKdUser[4]='\0'; ///ambil posisi karakter untuk kode_user dari 1baris teks itu
            break;
            }
        ke++;
        }
    }
    fclose(fdt);
}
//Untuk Login

void mainmenu() {
    clrscr();
    User Users[JmlDtUser+BufferDtUser];
    int pilihan=0;
    while (1) {
        clrscr(); printf("\n"); 
        printf(_BY " |=====================================================| \n");
        printf(" |            A   P   L   I   K   A   S   I            | \n");
        printf(" |          SISTEM PENGELOLAAN DATA KESEHATAN          | \n");
        printf(" |=====================================================| \n" _N);
        printf(_BM "                |-----------------------|             \n");
        printf("        "); printf(_BGG "  "_N);   printf(_BM "      | (1) Login             |      "); printf(_BGG "  "_N);      printf("    \n");
        printf("      "); printf(_BGG "      "_N); printf(_BM "    | (2) Daftar Akun Baru  |    ");    printf(_BGG "      "_N);  printf("    \n");
        printf("        "); printf(_BGG "  "_N);   printf(_BM "      | (3) Exit              |      "); printf(_BGG "  "_N);      printf("    \n");
        printf(_BM "                |-----------------------|             \n");
        printf(_BB "\n\n                 Pilihan: " _N); scanf("%i",&pilihan);

        switch (pilihan) {
        case 1:
            clear_input_buffer();
            login();
            if (LogLev == 1) {
                menuAdmin(Users);
            } else if (LogLev == 2) {
                menuUser(Users);
            } else { 
                printf(_NR "\n  User atau Password Salah atau Akun Belum Terdaftar. Tekan Enter untuk mencoba lagi!" _N);
                getchar();
            }
            break;
        case 2:
            clear_input_buffer();
            DaftarAkunBaru(Users);
            break;
        case 3:
            exit(0);
        default:
            printf(_NR " Pilihan tidak valid! Tekan Enter untuk mencoba lagi..."_N);
            getchar(); getchar();
        }
    } 
}

int main() {
    clrscr(); printf("\n\n");

    printf(_NG "  Periksa File Data..\n" _N );
    JmlDtUser=PeriksaFileData(FlUser); sleep(2);
    
    Aktivitas AktivitasBMR[4];
    strcpy(AktivitasBMR[0].aktivitas,"Sedentari (Tidak Aktif)         ");
    strcpy(AktivitasBMR[0].kdaktiv,"1"); AktivitasBMR[0].kdaktiv[1]='\0';
    strcpy(AktivitasBMR[1].aktivitas,"Ringan (Olahraga 1-3hari/minggu)");
    strcpy(AktivitasBMR[1].kdaktiv,"2"); AktivitasBMR[1].kdaktiv[1]='\0';
    strcpy(AktivitasBMR[2].aktivitas,"Sedang (Olahraga 3-5hari/minggu)");
    strcpy(AktivitasBMR[2].kdaktiv,"3"); AktivitasBMR[2].kdaktiv[1]='\0';
    strcpy(AktivitasBMR[3].aktivitas,"Berat (Olahraga 6-7hari/minggu  ");
    strcpy(AktivitasBMR[3].kdaktiv,"4"); AktivitasBMR[3].kdaktiv[1]='\0';
    
    mainmenu();
}




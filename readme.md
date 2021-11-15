
Zadani
==========
* Součástí bude shell s gramatikou cmd, tj. včetně exit
* prikazy a programy: echo, cd, dir, md, rd, type, find /v '' /c, sort, tasklist, shutdown
* program **rgen** (nahodne vygenerovana cislo v plovouci carce na stdout, dokud neprijde EOF(end of file), ETX(end of text) nebo EOT(end of transmission)) a **freq** (cte z stdin a sestavi frekvencni tabulku bytu, kterou pak vypise pro vsechny byty s frekvenci vetsi nez 0 ve formatu  “0x%hhx : %d”)
* roury a presmerovani
* simulovany disk se souborovym systemem FAT12 (soucasti balicku je disketa FreeDOS 1.2. Pripojeni v DOSBoxu pomoci:  **imgmount d fdos1_2_floppy.img -size 512,63,16,1**)

Projekt **boot**
==========
Tvorba filesystemu


* idt.h/cpp - interrupt descriptor table, tabulku preruseni. Tabulka uz je vyplnena nebo ji musime nejak vyplnit?
* disk.h/cpp - Čtení/Zápis sektorů na disku a další srajdy. Určitě se použije
* cmos.h/cpp - nejaka pocatecni konfigurace systemu a disku? CMOS pamet. Nebudeme editovat?
* keyboard.h/cpp - handling bufferu klavesnice. Asi nic k editovani pro nas?
* SimpleIni.h - Knihovna pro cteni .ini souboru. Takovy soubor mame v compiled/checker.ini
* vga.h/cpp - zobrazovaci techniky. V nasem pripade pouze vypis znakuuu i guess


Projekt **kernel**
==========
Jadro systemu:
1. roury a presmerovani
2. prikaz tasklist (vsechny procesy)
3. prikaz echo (echo musi umet @echo on a off)
4. program rgen
5. program freq


Monoliticke jadro.


* handles.h/cpp - handlery na pristupy ke knihovnam a k dalsim bs?
* io.h/cpp - "Read_Line_From_Console": volani VGA handleru a dalsi handling chyb, celkove zpracovani vstupu/vystupu uzivatele
* kernel.h/cpp - Inicializace/shutdown jadra, handling systemoveho volani, Bootstrap loader (Bootloader) => zavedeni programu systemu(pro nas user.dll), spusteni shellu

Projekt **user**
==========
Userspace

??????? pravdepodobne ma delat vsechny prikazy?
1. prikazy: cd, dir, md, type(typ predaneho souboru), search /v "" /c (hledani stringu v souboru/souborech), sort (serazeni radek vstupu podle abecedy(mozna i jinak?)),
2. Absolutni a relativni cesty
3. dir s parametrem /s => zobrazeni vsech souboru ve slozce a vsech dalsich v podslozkach


* shell.h/cpp - veskera implementace prikazu a definice shellu + parsing prikazu
* rtl.h/cpp - vyuziti systemoveho volani pro cteni a zapis souboru



Sdilene zdrojaky, ktere se nemeni (pravdepodobne):
==========

* hal.h  => Hardware abstraction layer (classic registry {rax, rbx, rcx, rdx}, index registr {rdi} - slouzi pro pomocne vypocty asi, flags, enum tridy obsahujici pravdepodobne vsechny potrebne hodnoty, info o disku (cylindry, cteci hlava .....))
* api.cpp/h => melo by poskytovat veskere potrebne informace, ostatni informace z filesystemu (napr predem stanovene adresare 0:\procfs). Obsahuje mozne odpovedi/zadosti API ve forme enum trid

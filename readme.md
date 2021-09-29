
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
* disk.h/cpp - cteni ze vstupu klavesnice uzivatele => volani preruseni. Asi nic zajimaveho/podstatneho?
* cmos.h/cpp - nejaka pocatecni konfigurace systemu a disku? CMOS pamet. Nebudeme editovat?
* keyboard.h/cpp -
* Simplelni.h - WTF is this. 3445 radek poradneho naseru. Nic pro nas?
* vga.h/cpp - zobrazovaci techniky. V nasem pripade pouze vypis znakuuu


Projekt **kernel**
==========
Jadro systemu:

* roury a presmerovani
* prikaz tasklist (vsechny procesy)
* prikaz echo (echo musi umet @echo on a off)

* program rgen
* program freq

* handles.h/cpp -
* io.h/cpp -
* kernel.h/cpp -

Projekt **user**
==========
Userspace

??????? pravdepodobne ma delat vsechny prikazy?
* prikazy: cd, dir, md, type(typ predaneho souboru), search /v "" /c (hledani stringu v souboru/souborech), sort (serazeni radek vstupu podle abecedy(mozna i jinak?)),
* Absolutni a relativni cesty
* dir s parametrem /s => zobrazeni vsech souboru ve slozce a vsech dalsich v podslozkach


* shell.h/cpp - veskera implementace prikazu a definice shellu + parsing prikazu
* rtl.h/cpp - vyuziti systemoveho volani pro cteni a zapis souboru



Sdilene zdrojaky, ktere se nemeni (pravdepodobne):
==========

* hal.h  => Hardware abstraction layer (classic registry {rax, rbx, rcx, rdx}, index registr {rdi} - slouzi pro pomocne vypocty asi, flags, enum tridy obsahujici pravdepodobne vsechny potrebne hodnoty, info o disku (cylindry, cteci hlava .....))
* api.cpp/h => melo by poskytovat veskere potrebne informace, ostatni informace z filesystemu (napr predem stanovene adresare 0:\procfs). Obsahuje mozne odpovedi/zadosti API ve forme enum trid





Repository pro inspiraci:
https://github.com/qwertzdenek/os-project

https://github.com/danisik/OS

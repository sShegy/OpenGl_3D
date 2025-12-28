# 3D Rubikova Kocka - Dokumentacija

## Opis projekta
Projekat je interaktivna 3D simulacija Rubikove kocke napisana u programskom jeziku C koristeći OpenGL 3.3.

## Korišćene tehnike
- **OpenGL 3.3 (Core Profile)**: Renderovanje geometrije kocke.
- **GLSL šejderi**: Prilagođeni vertex i fragment šejderi za senčenje i transformacije.
- **cglm biblioteka**: Korišćena za rad sa matricama i vektorima (MVP matrice).
- **Animacija**: Svaki sloj kocke se animira prilikom rotacije koristeći interpolaciju uglova.
- **Interakcija**:
    - **Miš**: Rotacija cele kocke u prostoru (drži levi klik i pomera miš).
    - **Tastatura**: Kontrola rotacije slojeva:
        - `U`: Upper layer
        - `D`: Down layer
        - `L`: Left layer
        - `R`: Right layer
        - `F`: Front layer
        - `B`: Back layer
        - `S`: Nasumično mešanje kocke (Scramble)
        - `ESC`: Izlaz iz aplikacije

## Struktura projekta
- `src/main.c`: Glavna logika aplikacije, inicijalizacija, render petlja i input.
- `res/shaders/`: Folder sa vertex (`vert.glsl`) i fragment (`frag.glsl`) šejderima.
- `include/cglm/`: Biblioteka za linearnu algebru.

## Pokretanje
Projekt se kompajlira pomoću CMake-a. Izvršni fajl se nalazi u `cmake-build-debug/RG25D2_Ime_Prezime`.

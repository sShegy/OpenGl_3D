# 3D Rubikova Kocka

## Opis projekta

Projekat predstavlja interaktivnu 3D simulaciju Rubikove kocke implementiranu u programskom jeziku **C** korišćenjem **OpenGL 3.3 Core Profile**. Aplikacija omogućava korisniku da rotira celu kocku, rotira pojedinačne slojeve, nasumično izmeša kocku (scramble) i vrati poteze unazad (solve).

Cilj projekta je demonstracija rada sa modernim OpenGL-om, shaderima, matricama transformacije, teksturama i osnovnom 3D interakcijom.

---

## Korišćene tehnologije i biblioteke

- **OpenGL 3.3 (Core Profile)**  
  Renderovanje 3D geometrije i rad sa GPU-om.

- **GLFW**  
  Kreiranje prozora, OpenGL konteksta i obrada korisničkog inputa (tastatura i miš).

- **GLAD**  
  Učitavanje OpenGL funkcija.

- **cglm**  
  Matematička biblioteka za rad sa vektorima i matricama (model, view, projection).

- **stb_image**  
  Učitavanje tekstura i cubemap (skybox).

- **GLSL (vertex i fragment shaderi)**  
  Transformacije, osvetljenje i teksturisanje.

---

## Funkcionalnosti

### 1. Prikaz Rubikove kocke
- Kocka se sastoji od **27 manjih kockica (cubies)** raspoređenih u 3×3×3 mrežu.
- Svaka kockica ima obojene strane koje predstavljaju boje Rubikove kocke.
- Kocka je blago skalirana kako bi se jasno videli razmaci između kockica.

### 2. Animirana rotacija slojeva
- Svaki potez se animira postepeno (rotacija od 0° do 90°).
- Rotacije su moguće oko X, Y i Z ose.
- Tokom animacije ostali inputi su blokirani kako bi se izbegle greške.

### 3. Interakcija mišem
- Držanjem **levog tastera miša** i pomeranjem:
    - Rotira se cela Rubikova kocka (yaw i pitch).
- Ograničen je vertikalni ugao rotacije kako bi se izbegao „flip“ kamere.

### 4. Tastatura – kontrole slojeva

| Taster | Akcija |
|------|------|
| **U** | Gornji sloj (Upper) |
| **D** | Donji sloj (Down) |
| **L** | Levi sloj (Left) |
| **R** | Desni sloj (Right) |
| **F** | Prednji sloj (Front) |
| **B** | Zadnji sloj (Back) |
| **S** | Nasumično mešanje (Scramble) |
| **SPACE** | Vraćanje poteza (Solve / Undo) |
| **ESC** | Izlaz iz aplikacije |

---

## Logika Rubikove kocke

- Svaki potez se pamti u **istoriji poteza (stack)**.
- Prilikom `Scramble` opcije generiše se niz nasumičnih rotacija.
- Opcija `Solve` koristi istoriju poteza i izvršava ih unazad (undo mehanizam).
- Rotacije se vrše matricama transformacije nad pojedinačnim kockicama.

> Napomena: Projekat se fokusira na **vizuelnu i interaktivnu simulaciju**. Logička permutacija boja ne simulira u potpunosti matematičko stanje prave Rubikove kocke, ali je vizuelno ponašanje ispravno.

---

## Skybox

- Implementiran je **cubemap skybox** koji okružuje scenu.
- Skybox se renderuje bez translacije kamere kako bi uvek ostao statičan u pozadini.
- Korišćeno je šest tekstura (right, left, top, bottom, front, back).
- Prilikom renderovanja skybox-a koristi se:
  ```c
  glDepthFunc(GL_LEQUAL);

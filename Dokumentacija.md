# 🎲 3D Rubikova Kocka (OpenGL)

![C](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3-red?style=for-the-badge&logo=opengl&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=for-the-badge)

> **Interaktivna 3D simulacija Rubikove kocke sa animacijama, zvukom i logikom za automatsko rešavanje.**

---

## 📖 Opis Projekta

Ovaj projekat je implementacija Rubikove kocke u programskom jeziku **C** koristeći **OpenGL 3.3 Core Profile**. Cilj je bio kreirati vizuelno dopadljivu i potpuno funkcionalnu simulaciju koja demonstrira napredne grafičke tehnike kao što su matrične transformacije, skybox renderovanje i rad sa audio sistemom.

Kocka nije statičan model – ona se sastoji od **27 proceduralno generisanih manjih kockica** koje se kreću nezavisno u prostoru.

---

## 🛠️ Korišćene Tehnologije

| Biblioteka | Namena |
| :--- | :--- |
| **OpenGL 3.3** | Renderovanje grafike (Core Profile) |
| **GLFW** | Upravljanje prozorom i inputima (tastatura/miš) |
| **GLAD** | Učitavanje OpenGL pointera |
| **cglm** | Napredna matematika (matrice, vektori, kvaternioni) |
| **stb_image** | Učitavanje tekstura i Skybox-a |
| **miniaudio** | Audio engine za zvučne efekte |

---

## ✨ Funkcionalnosti

*   🎨 **3D Renderovanje:** Realističan prikaz kocke sa teksturama.
*   🔄 **Animacije:** Glatke interpolirane rotacije slojeva.
*   🌌 **Skybox:** Imersivno 3D okruženje (Cubemap).
*   🔊 **Zvuk:** Zvučni efekti prilikom svakog poteza.
*   🧠 **Logika:**
    *   **Shuffle:** Nasumično mešanje kocke.
    *   **Auto-Solve:** Pamćenje poteza i automatsko rešavanje unazad.
*   🖱️ **Kamera:** Potpuna kontrola kamere mišem (Orbit system).

---

## 🎮 Kontrole

### 🖱️ Miš
*   **Drži Levi Klik + Pomeraj:** Rotacija kamere oko kocke.

### ⌨️ Tastatura (Rotacija Slojeva)

| Taster | Akcija (Sloj) |
| :---: | :--- |
| **I** | Gornji sloj (**Up**) |
| **K** | Donji sloj (**Down**) |
| **J** | Levi sloj (**Left**) |
| **L** | Desni sloj (**Right**) |
| **U** | Prednji sloj (**Front**) |
| **O** | Zadnji sloj (**Back**) |

### ⚙️ Funkcije Igre

| Taster | Funkcija |
| :---: | :--- |
| **S** | **Shuffle:** Nasumično mešanje kocke |
| **SPACE** | **Auto-Solve:** Automatsko rešavanje kocke |
| **H** | **Help:** Prikaz pomoći u konzoli |
| **ESC** | Izlaz iz programa |

---

## 🚀 Kako Pokrenuti

1.  Klonirajte repozitorijum.
2.  Uverite se da imate instalirane potrebne biblioteke ili da su linkovane u projektu (`lib` folder).
3.  Folder `res` (resursi) mora biti u istom direktorijumu kao i izvršni `.exe` fajl.
4.  Kompajlirajte koristeći vaš omiljeni C kompajler.

**Primer za GCC:**
```bash
gcc main.c glad.c -o rubik -lglfw3 -lgdi32 -lopengl32 -lm
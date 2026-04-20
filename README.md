<p align="center">
  <img src="assets/header.png" alt="Survivor Arsenal Header" width="600">
</p>

# 🚀 Survivor Arsenal

[![C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![SFML](https://img.shields.io/badge/Library-SFML-green.svg)](https://www.sfml-dev.org/)
[![Status](https://img.shields.io/badge/Status-Optimized-orange.svg)](#)

**Survivor Arsenal** es un trepidante juego de acción estilo *Bullet Hell* inspirado en clásicos como Contra y juegos modernos tipo *Survivor*. Enfrenta oleadas infinitas de enemigos, tanques y poderosos jefes mientras mejoras tu arsenal en tiempo real.

---

## ✨ Características Principales

- **🎮 Gameplay Adictivo:** Movimiento fluido y disparos multidireccionales.
- **👾 Variedad de Enemigos:** Desde soldados normales hasta tanques blindados y jefes mecánicos gigantes.
- **🆙 Sistema de Nivel de Experiencia:** Recoge gemas para subir de nivel y desbloquear mejoras:
  - 🔫 **Más Balas:** Incrementa tu cadencia de fuego.
  - ⚡ **Velocidad:** Aumenta tu agilidad para esquivar ataques.
  - 💉 **Curación:** Recupera tu salud en momentos críticos.
- **🏆 Récords:** Sistema de puntuación con persistencia de High Score.
- **🎨 Estética Retro:** Gráficos pixel art con una banda sonora dinámica.

---

## 🕹️ Controles

| Acción | Tecla |
| :--- | :--- |
| **Movimiento** | `W` `A` `S` `D` |
| **Disparar / Apuntar** | `Flechas Dirección` |
| **Empezar Juego** | `Espacio` (en el menú) |
| **Seleccionar Mejora** | `1`, `2` o `3` |
| **Volver al Menú** | `Esc` (tras Game Over) |

---

## 🛠️ Instalación y Reffigeración

### Requisitos Previos

Necesitarás tener instalada la librería **SFML** en tu sistema.

**En Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libsfml-dev
```

### Compilación

Puedes compilar el proyecto usando `g++`:

```bash
g++ main.cpp -o survivor_arsenal -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

### Ejecución

```bash
./survivor_arsenal
```

---

## 📂 Estructura del Proyecto

- `main.cpp`: Código fuente principal optimizado.
- `assets/`: 
  - `textures/`: Todas las imágenes y sprites (.png).
  - `audio/`: Efectos de sonido y música (.wav, .ogg).
  - `fonts/`: Archivos de tipografía (.ttf).
- `highscore.txt`: Archivo de persistencia de puntajes.

---

<p align="center">
  Desarrollado con ❤️ y C++
</p>

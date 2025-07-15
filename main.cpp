#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <SDL_mixer.h>

// Dimensiones de la ventana
const int ANCHO_VENTANA = 1200;
const int ALTURA_VENTANA = 650;

// Dimensiones de las paletas y la pelota
const int ANCHO_PALETA = 15;
const int ALTURA_PALETA = 90;
const int TAMANIO_PELOTA = 10;


// Velocidades
const int VELOCIDAD_PALETA = 400; // Píxeles por segundo
const int VELOCIDAD_PELOTA = 500;   // Píxeles por segundo


// Límite de puntuación para ganar
const int PUNTAJE_GANADOR = 5;


// Estados del eventoJuego
enum estadoJuego { MENU, INSTRUCCIONES, JUGANDO, GAME_OVER, SALIR };


struct paleta {
    float x, y;
    int puntaje;
};


struct pelota {
    float x, y;
    float vx, vy;
};


// Estructura del juego base y menu
struct pong {
    SDL_Window* ventana; // Formato de la ventana (tamaño, altura, fondo, etc.)
    SDL_Renderer* renderizar; // Funcion que renderiza todo
    TTF_Font* fuente; // Fuente del texto
    paleta paletaIzquierda; 
    paleta paletaDerecha;
    pelota pelota;  
    estadoJuego estadoDeJuego; // Estado en el que se encuentra el eventoJuego
    int opcionSeleccionada; // Selecciona la opcion correspondiente
    bool juegoIniciado; // Palanca para iniciar el eventoJuego
    Uint32 lastTime; // Escencial para que el juego vaya fluido 
    std::string mensajeGanador; // Almacena el mensaje del ganador
    Mix_Chunk* sonidoRebote = nullptr; // sonido molesto
    Mix_Chunk* sonidoPunto = nullptr; // Sondio ganador
    Mix_Music* musicaFondo = nullptr; // Musica bolviana
    SDL_Texture* fondo = nullptr; // Imagen del fondo de la pantalla
    SDL_Texture* sableIzquierdo = nullptr; // Imagen de sabel de luz izquierdo
    SDL_Texture* sableDerecho = nullptr; // Imagen de sable de luz derecho
}; 


// Función para inicializar SDL, ventana, renderizador y fuente
bool inicializarJuego(pong& juego) {
    
    // Inicia SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error inicializando SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Iniciar TTF
    if (TTF_Init() < 0) {
        std::cerr << "Error inicializando SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

   // Crear ventana del juego
    juego.ventana = SDL_CreateWindow(
        "Pong",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        ANCHO_VENTANA,
        ALTURA_VENTANA,
        SDL_WINDOW_SHOWN
    );
    if (!juego.ventana) {
        std::cerr << "Error creando ventana: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Renderizar el juego
    juego.renderizar = SDL_CreateRenderer(juego.ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!juego.renderizar) {
        std::cerr << "Error creando renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(juego.ventana);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Fuente principal del juego
    juego.fuente = TTF_OpenFont("assets/arial.ttf", 24);
    if (!juego.fuente) {
        std::cerr << "Error cargando fuente: " << TTF_GetError() << ". Asegurate de que arial.ttf este en la carpeta del ejecutable." << std::endl;
        SDL_DestroyRenderer(juego.renderizar);
        SDL_DestroyWindow(juego.ventana);
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    // Inicializar SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Error al inicializar SDL_mixer: " << Mix_GetError() << std::endl;
        return false;
    }


    juego.sonidoRebote = Mix_LoadWAV("assets/rebote_laser.wav");
    juego.sonidoPunto = Mix_LoadWAV("assets/gol_grito.wav");
    juego.musicaFondo = Mix_LoadMUS("assets/musica_starwars_inspirada.wav");



    if (!juego.sonidoRebote) {
        std::cerr << "Error cargando rebote_laser.wav: " << Mix_GetError() << std::endl;
    }
    if (!juego.sonidoPunto) {
        std::cerr << "Error cargando gol_grito.wav: " << Mix_GetError() << std::endl;
    }
    if (!juego.musicaFondo) {
        std::cerr << "Error cargando musica_starwars_inspirada.wav: " << Mix_GetError() << std::endl;
    }

    if (!juego.sonidoRebote || !juego.sonidoPunto || !juego.musicaFondo) return false;

    // Cargar efectos de sonido y música
    


   
    SDL_Surface* fondoSurface = SDL_LoadBMP("assets/star-wars-fondo.bmp");
    SDL_Surface* sableIzquierdoSurface = SDL_LoadBMP("assets/sable-rojo.bmp");
    SDL_Surface* sableDerechoSurface = SDL_LoadBMP("assets/sable-azul.bmp");

    if (!fondoSurface || !sableIzquierdoSurface || !sableDerechoSurface) {
        std::cerr << "Error cargando imagenes: " << SDL_GetError() << std::endl;
        return false;
    }

    // Cargar imagenes y texturas
    juego.fondo = SDL_CreateTextureFromSurface(juego.renderizar, fondoSurface);
    juego.sableIzquierdo = SDL_CreateTextureFromSurface(juego.renderizar, sableIzquierdoSurface);
    juego.sableDerecho = SDL_CreateTextureFromSurface(juego.renderizar, sableDerechoSurface);

    SDL_FreeSurface(fondoSurface);
    SDL_FreeSurface(sableIzquierdoSurface);
    SDL_FreeSurface(sableDerechoSurface);

    // Reproducir música de fondo en bucle
    Mix_PlayMusic(juego.musicaFondo, -1);

    // Inicializar paletas
    juego.paletaIzquierda = { 50, ALTURA_VENTANA / 2 - ALTURA_PALETA / 2, 0 };
    juego.paletaDerecha = { ANCHO_VENTANA - 50 - ANCHO_PALETA, ALTURA_VENTANA / 2 - ALTURA_PALETA / 2, 0 };

    // Inicializar pelota
    juego.pelota = { ANCHO_VENTANA / 2, ALTURA_VENTANA / 2, VELOCIDAD_PELOTA, VELOCIDAD_PELOTA };

    // Estado inicial
    juego.estadoDeJuego = MENU;
    juego.opcionSeleccionada = 0;
    juego.juegoIniciado = true;
    juego.lastTime = SDL_GetTicks();
    juego.mensajeGanador = "";

    return true;
} 


// Función para renderizar texto  
void renderizarTexto(SDL_Renderer* renderizador, TTF_Font* fuente, const std::string& texto, int x, int y, SDL_Color color) {   
   
    // Cargar fuentes
    if (!fuente) {
        std::cerr << "Error: Fuente no cargada para renderizar texto." << std::endl;
        return;
    } 

    // Renderizar fuente
    SDL_Surface* surface = TTF_RenderText_Solid(fuente, texto.c_str(), color);
    if (!surface) {
        std::cerr << "Error renderizando texto: " << TTF_GetError() << std::endl;
        return;
    }
    
    // Crea la textura del texto
    SDL_Texture* textura = SDL_CreateTextureFromSurface(renderizador, surface);
    if (!textura) {
        std::cerr << "Error creando textura de texto: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }


    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderizador, textura, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(textura);
}   


// Función para manejar eventos
void manejarEventos(pong& eventoJuego) {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        
        // Sale del juego
        if (evento.type == SDL_QUIT) {
            eventoJuego.estadoDeJuego = SALIR;
        }

        // Abre el menu
        else if (evento.type == SDL_KEYDOWN) {
            if (eventoJuego.estadoDeJuego == MENU) {
                if (evento.key.keysym.sym == SDLK_UP) {
                    eventoJuego.opcionSeleccionada = (eventoJuego.opcionSeleccionada - 1 + 3) % 3;
                }
                else if (evento.key.keysym.sym == SDLK_DOWN) {
                    eventoJuego.opcionSeleccionada = (eventoJuego.opcionSeleccionada + 1) % 3;
                }


                else if (evento.key.keysym.sym == SDLK_RETURN) {
                    if (eventoJuego.opcionSeleccionada == 0) {
                        eventoJuego.estadoDeJuego = JUGANDO;
                        // Reiniciar puntuaciones y pelota al iniciar el eventoJuego
                        eventoJuego.paletaIzquierda.puntaje = 0;
                        eventoJuego.paletaDerecha.puntaje = 0;
                        eventoJuego.pelota.x = ANCHO_VENTANA / 2;
                        eventoJuego.pelota.y = ALTURA_VENTANA / 2;
                        eventoJuego.pelota.vx = VELOCIDAD_PELOTA;
                        eventoJuego.pelota.vy = VELOCIDAD_PELOTA;
                        eventoJuego.mensajeGanador = "";
                    }
                    else if (eventoJuego.opcionSeleccionada == 1) eventoJuego.estadoDeJuego = INSTRUCCIONES;
                    else if (eventoJuego.opcionSeleccionada == 2) eventoJuego.estadoDeJuego = SALIR;
                }
            }
            else if (eventoJuego.estadoDeJuego == INSTRUCCIONES && evento.key.keysym.sym == SDLK_ESCAPE) {
                eventoJuego.estadoDeJuego = MENU;
            }
            else if (eventoJuego.estadoDeJuego == JUGANDO && evento.key.keysym.sym == SDLK_ESCAPE) {
                eventoJuego.estadoDeJuego = MENU; // Volver al menú desde el juego
            }
            else if (eventoJuego.estadoDeJuego == GAME_OVER && evento.key.keysym.sym == SDLK_RETURN) {
                eventoJuego.estadoDeJuego = MENU; // Volver al menú desde pong Over
            }
        }
    }
} 


// Función para actualizar la lógica del juego
void actualizarJuego(pong& actualizarJuego, float tiempoTranscurridoEntreFrames) {
    if (actualizarJuego.estadoDeJuego != JUGANDO) return;

    const Uint8* estadoDelTeclado = SDL_GetKeyboardState(NULL);

    // Mover paleta izquierda (W/S)
    if (estadoDelTeclado[SDL_SCANCODE_W] && actualizarJuego.paletaIzquierda.y > 0) {
        actualizarJuego.paletaIzquierda.y -= VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }
    if (estadoDelTeclado[SDL_SCANCODE_S] && actualizarJuego.paletaIzquierda.y < ALTURA_VENTANA - ALTURA_PALETA) {
        actualizarJuego.paletaIzquierda.y += VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }
    
    // Movimiento horizontal izquierda (A/D)
    if (estadoDelTeclado[SDL_SCANCODE_A] && actualizarJuego.paletaIzquierda.x > 0) {
        actualizarJuego.paletaIzquierda.x -= VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }
    if (estadoDelTeclado[SDL_SCANCODE_D] && actualizarJuego.paletaIzquierda.x < ANCHO_VENTANA / 2 - ANCHO_PALETA) {
        actualizarJuego.paletaIzquierda.x += VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }

    // Mover paleta derecha (flechas arriba/abajo)
    if (estadoDelTeclado[SDL_SCANCODE_UP] && actualizarJuego.paletaDerecha.y > 0) {
        actualizarJuego.paletaDerecha.y -= VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }
    if (estadoDelTeclado[SDL_SCANCODE_DOWN] && actualizarJuego.paletaDerecha.y < ALTURA_VENTANA - ALTURA_PALETA) {
        actualizarJuego.paletaDerecha.y += VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }
    
    // Movimiento horizontal derecha (flechas izquierda/derecha)
    if (estadoDelTeclado[SDL_SCANCODE_LEFT] && actualizarJuego.paletaDerecha.x > ANCHO_VENTANA / 2) {
        actualizarJuego.paletaDerecha.x -= VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }
    if (estadoDelTeclado[SDL_SCANCODE_RIGHT] && actualizarJuego.paletaDerecha.x < ANCHO_VENTANA - ANCHO_PALETA) {
        actualizarJuego.paletaDerecha.x += VELOCIDAD_PALETA * tiempoTranscurridoEntreFrames;
    }

    // Mover pelota
    actualizarJuego.pelota.x += actualizarJuego.pelota.vx * tiempoTranscurridoEntreFrames;
    actualizarJuego.pelota.y += actualizarJuego.pelota.vy * tiempoTranscurridoEntreFrames;

    // Rebotar contra techo
    if (actualizarJuego.pelota.y <= 0) {
        actualizarJuego.pelota.y = 0;
        actualizarJuego.pelota.vy = -actualizarJuego.pelota.vy;
        Mix_PlayChannel(-1, actualizarJuego.sonidoRebote, 0);
    }

    // Rebotar contra piso
    else if (actualizarJuego.pelota.y >= ALTURA_VENTANA - TAMANIO_PELOTA) {
        actualizarJuego.pelota.y = ALTURA_VENTANA - TAMANIO_PELOTA;
        actualizarJuego.pelota.vy = -actualizarJuego.pelota.vy;
        Mix_PlayChannel(-1, actualizarJuego.sonidoRebote, 0);
    }

    // Colisión con paletas
    SDL_Rect rectaDePelota = { (int)actualizarJuego.pelota.x, (int)actualizarJuego.pelota.y, TAMANIO_PELOTA, TAMANIO_PELOTA };
    SDL_Rect rectaDePaletaIzquierda = { (int)actualizarJuego.paletaIzquierda.x, (int)actualizarJuego.paletaIzquierda.y, ANCHO_PALETA, ALTURA_PALETA };
    SDL_Rect rectaDePaletaDerecha = { (int)actualizarJuego.paletaDerecha.x, (int)actualizarJuego.paletaDerecha.y, ANCHO_PALETA, ALTURA_PALETA };

    if (SDL_HasIntersection(&rectaDePelota, &rectaDePaletaIzquierda)) {
        actualizarJuego.pelota.x = actualizarJuego.paletaIzquierda.x + ANCHO_PALETA;
        actualizarJuego.pelota.vx = -actualizarJuego.pelota.vx;
        Mix_PlayChannel(-1, actualizarJuego.sonidoRebote, 0);
    }
    else if (SDL_HasIntersection(&rectaDePelota, &rectaDePaletaDerecha)) {
        actualizarJuego.pelota.x = actualizarJuego.paletaDerecha.x - TAMANIO_PELOTA;
        actualizarJuego.pelota.vx = -actualizarJuego.pelota.vx;
        Mix_PlayChannel(-1, actualizarJuego.sonidoRebote, 0);
    }

    // Puntuación y reinicio de pelota
    if (actualizarJuego.pelota.x + TAMANIO_PELOTA < 0) {
        Mix_PlayChannel(-1, actualizarJuego.sonidoPunto, 0);
        actualizarJuego.paletaDerecha.puntaje++;
        actualizarJuego.pelota.x = ANCHO_VENTANA / 2;
        actualizarJuego.pelota.y = ALTURA_VENTANA / 2;
        actualizarJuego.pelota.vx = VELOCIDAD_PELOTA;
        actualizarJuego.pelota.vy = VELOCIDAD_PELOTA;

        if (actualizarJuego.paletaDerecha.puntaje >= PUNTAJE_GANADOR) {
            actualizarJuego.estadoDeJuego = GAME_OVER;
            actualizarJuego.mensajeGanador = "¡Jugador Derecho Gana!";
        }
    }
   
    // Todo lo relacionado al lado izquierdo
    else if (actualizarJuego.pelota.x > ANCHO_VENTANA) {
        Mix_PlayChannel(-1, actualizarJuego.sonidoPunto, 0);
        actualizarJuego.paletaIzquierda.puntaje++;
        std::cout << "Puntuación: Izquierda " << actualizarJuego.paletaIzquierda.puntaje << " - Derecha " << actualizarJuego.paletaDerecha.puntaje << std::endl;
        actualizarJuego.pelota.x = ANCHO_VENTANA / 2;
        actualizarJuego.pelota.y = ALTURA_VENTANA / 2;
        actualizarJuego.pelota.vx = -VELOCIDAD_PELOTA;
        actualizarJuego.pelota.vy = VELOCIDAD_PELOTA;

        if (actualizarJuego.paletaIzquierda.puntaje >= PUNTAJE_GANADOR) {
            actualizarJuego.estadoDeJuego = GAME_OVER;
            actualizarJuego.mensajeGanador = "¡Jugador Izquierdo Gana!";
        }
    }  
} 


// Función para renderizar el juego
void renderizarJuego(const pong& renderizarJuego) {
    // Dibujar fondo
    SDL_RenderCopy(renderizarJuego.renderizar, renderizarJuego.fondo, NULL, NULL);

    // Dibujar sable izquierdo
    SDL_Rect rectIzq = { (int)renderizarJuego.paletaIzquierda.x, (int)renderizarJuego.paletaIzquierda.y, ANCHO_PALETA, ALTURA_PALETA };
    SDL_RenderCopy(renderizarJuego.renderizar, renderizarJuego.sableIzquierdo, NULL, &rectIzq);

    // Dibujar sable derecho
    SDL_Rect rectDer = { (int)renderizarJuego.paletaDerecha.x, (int)renderizarJuego.paletaDerecha.y, ANCHO_PALETA, ALTURA_PALETA };
    SDL_RenderCopy(renderizarJuego.renderizar, renderizarJuego.sableDerecho, NULL, &rectDer);

    if (renderizarJuego.estadoDeJuego == MENU) {
        SDL_Color color = { 255, 255, 255, 255 }; // Blanco
        SDL_Color selectedColor = { 255, 255, 0, 255 }; // Amarillo

        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Pong", ANCHO_VENTANA / 2 - 50, 100, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Jugar", ANCHO_VENTANA / 2 - 50, 200, renderizarJuego.opcionSeleccionada == 0 ? selectedColor : color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Instrucciones", ANCHO_VENTANA / 2 - 50, 250, renderizarJuego.opcionSeleccionada == 1 ? selectedColor : color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Salir", ANCHO_VENTANA / 2 - 50, 300, renderizarJuego.opcionSeleccionada == 2 ? selectedColor : color);
    }
   
    // Fue seleccionada la opcion INSTRUCCIONES
    else if (renderizarJuego.estadoDeJuego == INSTRUCCIONES) {
        SDL_Color color = { 255, 255, 255, 255 };
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Instrucciones", ANCHO_VENTANA / 2 - 80, 100, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Jugador Izquierdo:", ANCHO_VENTANA / 2 - 80, 200, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "W: Subir", ANCHO_VENTANA / 2 - 80, 230, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "S: Bajar", ANCHO_VENTANA / 2 - 80, 260, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Jugador Derecho:", ANCHO_VENTANA / 2 - 80, 300, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Flecha Arriba: Subir", ANCHO_VENTANA / 2 - 80, 330, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Flecha Abajo: Bajar", ANCHO_VENTANA / 2 - 80, 360, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Presiona ESC para volver", ANCHO_VENTANA / 2 - 80, 450, color);
    }
    
    // Fue seleccionada la opcion JUGAR
    else if (renderizarJuego.estadoDeJuego == JUGANDO) {
        SDL_SetRenderDrawColor(renderizarJuego.renderizar, 255, 0, 0, 255); // Objetos blancos

        // Dibujar pelota
        SDL_Rect ballDraw = { (int)renderizarJuego.pelota.x, (int)renderizarJuego.pelota.y, TAMANIO_PELOTA, TAMANIO_PELOTA };
        SDL_RenderFillRect(renderizarJuego.renderizar, &ballDraw);

        // Mostrar puntuación
        std::string textoDelPuntaje = std::to_string(renderizarJuego.paletaIzquierda.puntaje) + " - " + std::to_string(renderizarJuego.paletaDerecha.puntaje);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, textoDelPuntaje, ANCHO_VENTANA / 2 - 20, 20, { 255, 255, 255, 255 });
    }
    
    // Se cerro el juego, sea por la opcion salir y/o gano un jugador
    else if (renderizarJuego.estadoDeJuego == GAME_OVER) {
        SDL_Color color = { 255, 255, 255, 255 };
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, renderizarJuego.mensajeGanador, ANCHO_VENTANA / 2 - 100, ALTURA_VENTANA / 2 - 50, color);
        renderizarTexto(renderizarJuego.renderizar, renderizarJuego.fuente, "Presiona ENTER para volver al menú", ANCHO_VENTANA / 2 - 100, ALTURA_VENTANA / 2 + 50, color);
    }

    SDL_RenderPresent(renderizarJuego.renderizar);
} 


// Función para limpiar recursos
void limpiarJuego(pong& limpiarJuego) {
    if (limpiarJuego.fuente) TTF_CloseFont(limpiarJuego.fuente);
    if (limpiarJuego.renderizar) SDL_DestroyRenderer(limpiarJuego.renderizar);
    if (limpiarJuego.ventana) SDL_DestroyWindow(limpiarJuego.ventana);
    if (limpiarJuego.fondo) SDL_DestroyTexture(limpiarJuego.fondo);
    if (limpiarJuego.sableIzquierdo) SDL_DestroyTexture(limpiarJuego.sableIzquierdo);
    if (limpiarJuego.sableDerecho) SDL_DestroyTexture(limpiarJuego.sableDerecho);
    if (limpiarJuego.sonidoRebote) Mix_FreeChunk(limpiarJuego.sonidoRebote);
    if (limpiarJuego.sonidoPunto) Mix_FreeChunk(limpiarJuego.sonidoPunto);
    if (limpiarJuego.musicaFondo) Mix_FreeMusic(limpiarJuego.musicaFondo);
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
} 


//Funcion principal para que funcione el juego, el cuerpo de todo el programa
int main(int argc, char* argv[]) {
    pong juegoFinal = {};

    // Inicializar el eventoJuego
        if (!inicializarJuego(juegoFinal)) {
            return 1;
        }

    // Bucle principal
    while (juegoFinal.juegoIniciado) {
        // Calcular tiempo transcurrido entre frames
        Uint32 fluidezDelJuego = SDL_GetTicks();
        float tiempoTranscurridoEntreFrames = (fluidezDelJuego - juegoFinal.lastTime) / 1000.0f;
        juegoFinal.lastTime = fluidezDelJuego;

        // Manejar eventos
        manejarEventos(juegoFinal);

        // Actualizar lógica
        actualizarJuego(juegoFinal, tiempoTranscurridoEntreFrames);

        // Renderizar
        renderizarJuego(juegoFinal);

        // Salir si está en estado EXIT
        if (juegoFinal.estadoDeJuego == SALIR) {
            juegoFinal.juegoIniciado = false;
        }
    }

    // Limpiar recursos
    limpiarJuego(juegoFinal);

    return 0;
} 
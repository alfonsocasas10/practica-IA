#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

// ./build/practica2 -m mapas/mapa30.map -n 0 -i 17 5 0 -t 17 17 0

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores)
{
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoIngenieroNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoIngenieroNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoIngenieroNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoIngenieroNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoIngenieroNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoIngenieroNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoIngenieroNivel_6(sensores);
    break;
  }

  return accion;
}

int ComportamientoIngeniero::veoCasillaInteresanteI0(char i, char c, char d, bool zaps){

  if (c == 'U') return 2;
  else if (d == 'U') return 3;
  else if (i == 'U') return 1;
  else if (!zaps){
    if (c == 'D') return 2;
    else if (d == 'D') return 3;
    else if (i == 'D') return 1;
  } 
  
  if (c == 'C'||c == 'D') return 2;
  else if (d == 'C'||c == 'D') return 3;
  else if (i == 'C'||c == 'D') return 1;

  return 0;
}

int ComportamientoIngeniero::veoCasillaInteresanteI1(char i, char c, char d, bool zaps){

  if (!zaps){
    if (c == 'D') return 2;
    else if (d == 'D') return 3;
    else if (i == 'D') return 1;
  } 
  
  else if (ComportamientoIngeniero::es_camino(c)) return 2;
  else if (ComportamientoIngeniero::es_camino(d)) return 3;
  else if (ComportamientoIngeniero::es_camino(i)) return 1;
  else if (c == 'S') return 2;
  else if (d == 'S') return 3;
  else if (i == 'S') return 1;
  else if (c == 'H') return 2;
  else if (d == 'H') return 3;
  else if (i == 'H') return 1;
  else if (c == 'A') return 2;
  else if (d == 'A') return 3;
  else if (i == 'A') return 1;

  return 0;
}

char ComportamientoIngeniero::viablePorAlturaI(char casilla, int dif, bool zap){

  if (abs(dif) <= 1 || (abs(dif) <= 2 && zap)) return casilla;
  else return 'P';
}

int veoCasillaExplorarI(bool vi, bool vc, bool vd,
                       char i, char c, char d) {

  if (!vc && (c == 'C' || c == 'U' || c == 'D')) return 2;
  else if (!vd && (d == 'C' || d == 'U' || d == 'D')) return 3;
  else if (!vi && (i == 'C' || i == 'U' || i == 'D')) return 1;

  return 0;
}

bool ComportamientoIngeniero::puedeSaltarI(const Sensores &sensores, bool tiene_zaps) {
  // Casilla intermedia: superficie[2], cota[2]
  // Casilla destino: superficie[6], cota[6]
  
  // 1. La casilla intermedia [2] debe ser transitable y NO tener un agente
  // (Asumiendo que es_camino ya filtra muros/precipicios)
  if (sensores.agentes[2] != '_' || sensores.superficie[2] == 'M' || sensores.superficie[2] == 'P' || sensores.superficie[2] == 'B') {
    return false;
  }

  // 2. La casilla destino [6] debe ser transitable
  if (!ComportamientoIngeniero::es_camino(sensores.superficie[6])) {
    return false;
  }

  // 3. Diferencia de altura entre origen [0] y destino [6]
  int diff_altura = abs(sensores.cota[6] - sensores.cota[0]);
  int limite = tiene_zaps ? 3 : 2;

  if (diff_altura >= limite) {
    return false;
  }

  return true;
}

// Niveles iniciales (Comportamientos reactivos simples)
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
{
  Action accion;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D') zaps = true;

  // 1. Inicializar la matriz la primera vez y sumar 1 a la casilla actual
  if (visitas.empty()) {
      visitas.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
  }
  visitas[sensores.posF][sensores.posC]++;

  // 2. Calcular las coordenadas de Frente, Izquierda y Derecha
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion pos_frente = Delante(actual);
  
  ubicacion aux_izq = actual;
  aux_izq.brujula = (Orientacion)(((int)aux_izq.brujula + 7) % 8);
  ubicacion pos_izq = Delante(aux_izq);
  
  ubicacion aux_der = actual;
  aux_der.brujula = (Orientacion)(((int)aux_der.brujula + 1) % 8);
  ubicacion pos_der = Delante(aux_der);

  // DETECTAR BLOQUEO
  if ((last_action == WALK) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // DETECTAR BLOQUEO
  if ((last_action == JUMP) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo_J = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // FILTRAR ALTURA
  char i = viablePorAlturaI(sensores.superficie[1],
                           sensores.cota[1] - sensores.cota[0], zaps);

  char c = viablePorAlturaI(sensores.superficie[2],
                           sensores.cota[2] - sensores.cota[0], zaps);

  char d = viablePorAlturaI(sensores.superficie[3],
                           sensores.cota[3] - sensores.cota[0], zaps);
                           
   int pos = veoCasillaInteresanteI0(i, c, d, zaps);

   if (sensores.superficie[0] == 'U') {
    if (c == 'U') return WALK;
    else if (d == 'U') return TURN_SR;
    else if (i == 'U') return TURN_SL;
    else{
      return IDLE;
    } 
  }

  // ================= BLOQUEO =================
  
  if ((en_bloqueo && c == 'U') || (en_bloqueo_J && sensores.superficie[6] == 'U')){
    en_bloqueo_U = true;
    contador_giros++;
    last_action = giro_preferido;
    return giro_preferido;
  }

  if (en_bloqueo_U){
    if (pos == 2 && en_bloqueo){
      en_bloqueo_U = false;
      en_bloqueo = false;
      accion = WALK;
    } 
    else if (en_bloqueo_J && puedeSaltarI(sensores, zaps)){
      en_bloqueo_U = false;
      en_bloqueo = false;
      accion = JUMP;
    }
    else{
      contador_giros++;
      accion = giro_preferido;
    }
    last_action = accion;
    return accion;
  }
  else if (en_bloqueo) {
    if (pos == 2){
      en_bloqueo = false;
      accion = WALK;
    } 
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
  else if (en_bloqueo_J) {

    if (pos == 2){
      en_bloqueo_J = false;
      accion = WALK;
    } 
    else if(puedeSaltarI(sensores, zaps)){
      en_bloqueo_J = false;
      accion = JUMP;
    }
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
  // --- LÓGICA DE SALTO ---
  // Si la meta 'U' está a dos casillas (pos 6) y podemos saltar con seguridad
  if (en_bloqueo && sensores.superficie[6] == 'U' && puedeSaltarI(sensores, zaps)) {
    last_action = JUMP;
    return JUMP;
  }
    // ================= NORMAL =================
  else {
    // 1. Si hay una 'U' a la vista, ignoramos la memoria y vamos a por ella
    if (c == 'U') {
      accion = WALK;
      giro_defecto = false;
    } 
    else if (d == 'U') {
      accion = TURN_SR;
      giro_defecto = false;
    } 
    else if (i == 'U') {
      accion = TURN_SL;
      giro_defecto = false;
    }
    // 2. SISTEMA DE MEMORIA (Solo entra si no hay 'U')
    else if (es_camino(sensores.superficie[1]) && (i != 'P') && visitas[pos_izq.f][pos_izq.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SL;
    }
    else if (es_camino(sensores.superficie[3]) && (d != 'P') && visitas[pos_der.f][pos_der.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SR;
    }
    // 3. MOVIMIENTO NORMAL (Si no hay 'U' y la memoria no pide girar)
    else if (pos == 2) {
      accion = WALK;
      giro_defecto = false;
    }
    else if (pos == 3) {
      accion = TURN_SR;
      giro_defecto = false;
    }
    else if (pos == 1) {
      accion = TURN_SL;
      giro_defecto = false;
    }
    else if (puedeSaltarI(sensores, zaps) && (pos == 0)) { 
    accion = JUMP;
    giro_defecto = false;
  }
    // 4. BLOQUEO / ROTACIÓN POR DEFECTO
    else {
      // probar ambos lados
      if (!giro_defecto){
        accion = giro_preferido;
        giro_defecto = true;
      }
      else if (giro_preferido == TURN_SL) {
         accion = TURN_SR;
      } else {
        accion = TURN_SL;
      }

      contador_giros++;
    }

    // romper ciclos
    if (contador_giros >= 15) {
      giro_preferido = (giro_preferido == TURN_SL) ? TURN_SR : TURN_SL;
      contador_giros = 0;
    }
  }

  if (accion == WALK){
    cont_walk++;
    if(cont_walk >= 5){
      if (walk_left){
        accion = TURN_SL;
        walk_left = false;
      } 
      else{
        accion = TURN_SR;
        walk_left = true;
      } 
      cont_walk = 0;
      contador_giros++;
    } 
  }
  else cont_walk = 0;

  last_f = sensores.posF;
  last_c = sensores.posC;
  last_action = accion;

  return accion;
}

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoIngeniero::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U');
}

bool ComportamientoIngeniero::es_camino1(unsigned char c) const 
{
  return (c == 'C' || c == 'D' || c == 'U' || c == 'H' || c == 'S' || c == 'A');
}

/**
 * @brief Comportamiento reactivo del ingeniero para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores)
{
  Action accion;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D') zaps = true;

  // 1. Inicializar la matriz la primera vez y sumar 1 a la casilla actual
  if (visitas.empty()) {
      visitas.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
  }
  visitas[sensores.posF][sensores.posC]++;

  // 2. Calcular las coordenadas de Frente, Izquierda y Derecha
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion pos_frente = Delante(actual);
  
  ubicacion aux_izq = actual;
  aux_izq.brujula = (Orientacion)(((int)aux_izq.brujula + 7) % 8);
  ubicacion pos_izq = Delante(aux_izq);
  
  ubicacion aux_der = actual;
  aux_der.brujula = (Orientacion)(((int)aux_der.brujula + 1) % 8);
  ubicacion pos_der = Delante(aux_der);

  // DETECTAR BLOQUEO
  if ((last_action == WALK) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // DETECTAR BLOQUEO
  if ((last_action == JUMP) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo_J = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // FILTRAR ALTURA
  char i = viablePorAlturaI(sensores.superficie[1],
                           sensores.cota[1] - sensores.cota[0], zaps);

  char c = viablePorAlturaI(sensores.superficie[2],
                           sensores.cota[2] - sensores.cota[0], zaps);

  char d = viablePorAlturaI(sensores.superficie[3],
                           sensores.cota[3] - sensores.cota[0], zaps);
                           
  int pos = veoCasillaInteresanteI1(i, c, d, zaps);

 // ================= BLOQUEO =================

  if (en_bloqueo) {
    if (es_camino1(sensores.superficie[2]) && sensores.superficie[2] == c){
      en_bloqueo = false;
      accion = WALK;
    } 
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
  else if (en_bloqueo_J) {

    if (es_camino1(sensores.superficie[2]) && sensores.superficie[2] == c){
      en_bloqueo_J = false;
      accion = WALK;
    } 
    else if(puedeSaltarI(sensores, zaps)){
      en_bloqueo_J = false;
      accion = JUMP;
    }
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
  // --- LÓGICA DE SALTO ---
  // Si la meta 'U' está a dos casillas (pos 6) y podemos saltar con seguridad
  
    // ================= NORMAL =================
  
    // 1. Si hay una 'U' a la vista, ignoramos la memoria y vamos a por ella
    if (es_camino1(sensores.superficie[1]) && (i != 'P') && visitas[pos_izq.f][pos_izq.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SL;
    }
    else if (es_camino1(sensores.superficie[3]) && (d != 'P') && visitas[pos_der.f][pos_der.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SR;
    }
    // 3. MOVIMIENTO NORMAL (Si no hay 'U' y la memoria no pide girar)
    else if (pos == 2) {
      accion = WALK;
      giro_defecto = false;
    }
    else if (pos == 3) {
      accion = TURN_SR;
      giro_defecto = false;
    }
    else if (pos == 1) {
      accion = TURN_SL;
      giro_defecto = false;
    }
    else if (puedeSaltarI(sensores, zaps) && (pos == 0)) { 
    accion = JUMP;
    giro_defecto = false;
    }
    // 4. BLOQUEO / ROTACIÓN POR DEFECTO
    else {
      // probar ambos lados
      if (!giro_defecto){
        accion = giro_preferido;
        giro_defecto = true;
      }
      else if (giro_preferido == TURN_SL) {
         accion = TURN_SR;
      } else {
        accion = TURN_SL;
      }

      contador_giros++;
    }


    // romper ciclos
    if (contador_giros >= 15) {
      giro_preferido = (giro_preferido == TURN_SL) ? TURN_SR : TURN_SL;
      contador_giros = 0;
    }
  

  if (accion == WALK){
    cont_walk++;
    if(cont_walk >= 5){
      if (walk_left){
        accion = TURN_SL;
        walk_left = false;
      } 
      else{
        accion = TURN_SR;
        walk_left = true;
      } 
      cont_walk = 0;
      contador_giros++;
    } 
  }
  else cont_walk = 0;

  last_f = sensores.posF;
  last_c = sensores.posC;
  last_action = accion;

  return accion;
}

// Niveles avanzados (Uso de búsqueda)
/**
 * @brief Comportamiento del ingeniero para el Nivel 2 (búsqueda).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
  // TODO: Implementar búsqueda para el Nivel 2.
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores)
{
  return IDLE;
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoIngeniero::ActualizarMapa(Sensores sensores)
{
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo)
  {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
        mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
      }
    break;
  case noreste:
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
    mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
    mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
    mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
    mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
    mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
    mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
    mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
    mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
    break;
  case este:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
        mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
      }
    break;
  case sureste:
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
    mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
    mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
    mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
    mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
    mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
    mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
    mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
    mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
    mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
    mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
    break;
  case sur:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
        mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
      }
    break;
  case suroeste:
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
    mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
    mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
    mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
    mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
    mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
    mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
    mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
    mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
    break;
  case oeste:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
        mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
      }
    break;
  case noroeste:
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
    mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
    mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
    mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
    mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
    mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
    mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
    mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
    mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
    mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
    mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
    break;
  }
}

/**
 * @brief Determina si una casilla es transitable para el ingeniero.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable (no es muro ni precipicio).
 */
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el ingeniero: desnivel máximo 1 sin zapatillas, 2 con zapatillas.
 * @param actual Estado actual del agente (fila, columna, orientacion, zap).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)
    return false;
  if (!zap && desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
  case 0:
    delante.f--;
    break; // norte
  case 1:
    delante.f--;
    delante.c++;
    break; // noreste
  case 2:
    delante.c++;
    break; // este
  case 3:
    delante.f++;
    delante.c++;
    break; // sureste
  case 4:
    delante.f++;
    break; // sur
  case 5:
    delante.f++;
    delante.c--;
    break; // suroeste
  case 6:
    delante.c--;
    break; // oeste
  case 7:
    delante.f--;
    delante.c--;
    break; // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    cout << it->fil << ", " << it->col << " (" << it->op << ")\n";
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st,
                                            const list<Action> &plan)
{
  listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan)
{
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end())
  {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}

#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

bool g_ingeniero_listo_install = false;
bool g_tecnico_listo_install = false;
vector<Paso> g_plan_n5;



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
  // En el caso del ingeniero he preferido ser mas conservador y no dejar que entre al agua ni a la hierba,
  // para hacer un equilibrio entre energía y mapa investigado, ya que cuenta con el salto y considero que 
  // es más conveniente hacerlo de esta manera.
  if (!zaps){
    if (c == 'D') return 2;
    else if (d == 'D') return 3;
    else if (i == 'D') return 1;
  } 
  
  if (ComportamientoIngeniero::es_camino(c)) return 2;
  else if (ComportamientoIngeniero::es_camino(d)) return 3;
  else if (ComportamientoIngeniero::es_camino(i)) return 1;
  else if (c == 'S') return 2;
  else if (d == 'S') return 3;
  else if (i == 'S') return 1;

  return 0;
}

char ComportamientoIngeniero::viablePorAlturaI(char casilla, int dif, bool zap){
  if (abs(dif) <= 1 || (abs(dif) <= 2 && zap)) return casilla;
  else return 'P';
}


bool ComportamientoIngeniero::puedeSaltarI(const Sensores &sensores, bool tiene_zaps) {
  // Si hay agente, muro, precipicio, o bosque en medio no puede saltar
  if (sensores.agentes[2] != '_' || sensores.superficie[2] == 'M' || sensores.superficie[2] == 'P' || sensores.superficie[2] == 'B') {
    return false;
  }

  // Confirmamos que la casilla destino es transitable
  if (!ComportamientoIngeniero::es_camino(sensores.superficie[6])) {
    return false;
  }

  // La diferencia debe ser correcta
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

  // Inicializo matriz que cuenta las veces que pasa por una casilla
  if (visitas.empty()) {
      visitas.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
  }
  // +1 en nuestra posicion
  visitas[sensores.posF][sensores.posC]++;

  // Calulamos coordenadas de las posciones 1, 2 ,3 (i, c, d)
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion pos_frente = Delante(actual);
  
  ubicacion aux_izq = actual;
  aux_izq.brujula = (Orientacion)(((int)aux_izq.brujula + 7) % 8);
  ubicacion pos_izq = Delante(aux_izq);
  
  ubicacion aux_der = actual;
  aux_der.brujula = (Orientacion)(((int)aux_der.brujula + 1) % 8);
  ubicacion pos_der = Delante(aux_der);

  // Detecta bloqueo por andar
  if ((last_action == WALK) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Detecta bloqueo por saltar
  if ((last_action == JUMP) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo_J = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Vemos qué opciones tienen altura correcta
  char i = viablePorAlturaI(sensores.superficie[1],
                           sensores.cota[1] - sensores.cota[0], zaps);

  char c = viablePorAlturaI(sensores.superficie[2],
                           sensores.cota[2] - sensores.cota[0], zaps);

  char d = viablePorAlturaI(sensores.superficie[3],
                           sensores.cota[3] - sensores.cota[0], zaps);
  
  // Nos quedamos con la posicion de la casilla que más nos interese
  int pos = veoCasillaInteresanteI0(i, c, d, zaps);

  // Si estamos en U, solo avanzamos en caso de tener otra U en un paso, sino nos mantenemos
  if (sensores.superficie[0] == 'U') {
    if (c == 'U') return WALK;
    else if (d == 'U') return TURN_SR;
    else if (i == 'U') return TURN_SL;
    else{
      return IDLE;
    } 
  }


  // Tratamos el bloqueo (creado para colisiones con tecnico en U)
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
  // Aquí tratamos un bloque por andar normal.
  else if (en_bloqueo) {
    if (pos == 2){
      en_bloqueo = false;
      accion = WALK;
    } 
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
  // Y aquí tratamos un bloqueo por saltar.
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
  // Aquí, en primer lugar, salto a U siempre que se pueda.
  if (en_bloqueo && sensores.superficie[6] == 'U' && puedeSaltarI(sensores, zaps)) {
    last_action = JUMP;
    return JUMP;
  }
  // Llegados aquí, no hay bloqueos
  else {
    // Si hay U accesible entra directo
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
    // En caso de no haber U, usamos el sistema de memoria para ir por las zonas menos visitadas (para romper bucles e 
    // investigar más hasta encontrar U)
    else if (es_camino(sensores.superficie[1]) && (i != 'P') && visitas[pos_izq.f][pos_izq.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SL;
    }
    else if (es_camino(sensores.superficie[3]) && (d != 'P') && visitas[pos_der.f][pos_der.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SR;
    }
    // Llegados aqui, simplemente optamos por la posicion elegida anteriormente
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
    // En caso de no haber salida, comprobamos si podemos saltar, y en caso de poder, saltamos
    else if (puedeSaltarI(sensores, zaps) && (pos == 0)) { 
    accion = JUMP;
    giro_defecto = false;
  }
    // Esto es para que primero gire para un lado, y si no hay salidas significa que por ese lado no había camino, y en ese punto giramos 
    // por completo hacia el otro por si habia un camino justo en ese lado. Sino hubiese, terminaría de dar la vuelta porque la salida estabaa detras
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

    // Variar giros, que no sea monótono el movimiento del agente durante toda la simulacion
    if (contador_giros >= 15) {
      giro_preferido = (giro_preferido == TURN_SL) ? TURN_SR : TURN_SL;
      contador_giros = 0;
    }
  }

  // Además, tras andar 5 veces, girará hacia un lado. Esto se hace para romper posibles bucles también.
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
  return (c == 'C' || c == 'D' || c == 'U' || c == 'S');
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

  // Inicializo matriz que cuenta las veces que pasa por una casilla
  if (visitas.empty()) {
      visitas.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
  }
  // +1 en nuestra posicion
  visitas[sensores.posF][sensores.posC]++;

  // Calulamos coordenadas de las posciones 1, 2 ,3 (i, c, d)
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion pos_frente = Delante(actual);
  
  ubicacion aux_izq = actual;
  aux_izq.brujula = (Orientacion)(((int)aux_izq.brujula + 7) % 8);
  ubicacion pos_izq = Delante(aux_izq);
  
  ubicacion aux_der = actual;
  aux_der.brujula = (Orientacion)(((int)aux_der.brujula + 1) % 8);
  ubicacion pos_der = Delante(aux_der);

  // Detectamos bloqueo por andar
  if ((last_action == WALK) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Detectamos bloqueo por saltar
  if ((last_action == JUMP) &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo_J = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Vemos si las alturas son correctas o no podemos acceder a alguna direccion
  char i = viablePorAlturaI(sensores.superficie[1],
                           sensores.cota[1] - sensores.cota[0], zaps);

  char c = viablePorAlturaI(sensores.superficie[2],
                           sensores.cota[2] - sensores.cota[0], zaps);

  char d = viablePorAlturaI(sensores.superficie[3],
                           sensores.cota[3] - sensores.cota[0], zaps);
    
  // Nos quedamos con la casilla que más nos interese
  int pos = veoCasillaInteresanteI1(i, c, d, zaps);

 // Gestionamos los bloqueos
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
  // En este punto no hay bloqueos, y priorizamos las zonas menos visitadas para investigar nuevos caminos
  if (es_camino1(sensores.superficie[1]) && (i != 'P') && visitas[pos_izq.f][pos_izq.c] < visitas[pos_frente.f][pos_frente.c]) {
    accion = TURN_SL;
    
  }
  else if (es_camino1(sensores.superficie[3]) && (d != 'P') && visitas[pos_der.f][pos_der.c] < visitas[pos_frente.f][pos_frente.c]) {
    accion = TURN_SR;
    
  }
  else if (es_camino1(sensores.superficie[2]) && (c != 'P') && sensores.superficie[2] == c) {
    accion = WALK;
    
  }
  // Llegados aquí, optamos por la casilla que más nos convenga elegida anteriormente
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
  // Aquí estamos sin salidas, si podemos saltar, saltamos
  else if (puedeSaltarI(sensores, zaps) && (pos == 0)) { 
  accion = JUMP;
  giro_defecto = false;
  
  }
  // Si no podemos saltar, giramos hasta encontrar salida
  else {
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


  // Rompemos ciclos
  if (contador_giros >= 15) {
    giro_preferido = (giro_preferido == TURN_SL) ? TURN_SR : TURN_SL;
    contador_giros = 0;
  }

  // Tambien para romper ciclos
  if (accion == WALK){
    cont_walk++;
    if(cont_walk >= 4){
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

ubicacion ComportamientoIngeniero::SimularAccionI(ubicacion actual, Action a) {
  ubicacion sig = actual;
  int salto = (a == JUMP) ? 2 : 1;

  if (a == WALK || a == JUMP) {
    switch (actual.brujula) {
      case norte:     sig.f -= salto; break;
      case noreste:   sig.f -= salto; sig.c += salto; break;
      case este:      sig.c += salto; break;
      case sureste:   sig.f += salto; sig.c += salto; break;
      case sur:       sig.f += salto; break;
      case suroeste:  sig.f += salto; sig.c -= salto; break;
      case oeste:     sig.c -= salto; break;
      case noroeste:  sig.f -= salto; sig.c -= salto; break;
    }
  } else if (a == TURN_SR) {
    sig.brujula = (Orientacion)((actual.brujula + 1) % 8);
  } else if (a == TURN_SL) {
    sig.brujula = (Orientacion)((actual.brujula + 7) % 8);
  }
  return sig;
}

bool ComportamientoIngeniero::CasillaTransitableI(int f, int c, int f_ant, int c_ant, bool tiene_zaps) {
  // PRIMERO: Validar límites para no morir en el intento
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) 
    return false;

  // SEGUNDO: Obstáculos físicos
  char celda = mapaResultado[f][c];
  if (celda == 'P' || celda == 'M' || celda == 'B' || celda == '?') return false;

  // TERCERO: Altura (aquí es donde se caía si f o c eran inválidos)
  int dif = abs((int)mapaCotas[f][c] - (int)mapaCotas[f_ant][c_ant]);
  int limite = tiene_zaps ? 3 : 2;

  return (dif < limite);
}


// Niveles avanzados (Uso de búsqueda)
/**
 * @brief Comportamiento del ingeniero para el Nivel 2 (búsqueda).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{

if (sensores.superficie[0] == 'D') {
    zaps = true;
  }

  if (!hayPlan) {
    plan.clear();
    forbidden_cells.clear();

    // Detectar al técnico usando sensores y marcar su posición como prohibida
    for (int i = 1; i < 16; i++) {
      if (sensores.agentes[i] != '_') {
        ubicacion pos_actual = {sensores.posF, sensores.posC, sensores.rumbo};
        ubicacion pos_agente;
        
        if (i == 2) {
          pos_agente = SimularAccionI(pos_actual, WALK);
        } else if (i == 6) {
          pos_agente = SimularAccionI(pos_actual, JUMP);
        } else {
          // Para otros sensores, calcular basándose en el rumbo 
          int df = 0, dc = 0;
          if (sensores.rumbo == 0) { // norte
            if (i == 1) { df = -1; dc = -1; }
            else if (i == 2) { df = -1; dc = 0; }
            else if (i == 3) { df = -1; dc = 1; }
            else if (i == 6) { df = -2; dc = 0; }
          } else if (sensores.rumbo == 2) { // este
            if (i == 1) { df = -1; dc = 1; }
            else if (i == 2) { df = 0; dc = 1; }
            else if (i == 3) { df = 1; dc = 1; }
            else if (i == 6) { df = 0; dc = 2; }
          } else if (sensores.rumbo == 4) { // sur
            if (i == 1) { df = 1; dc = 1; }
            else if (i == 2) { df = 1; dc = 0; }
            else if (i == 3) { df = 1; dc = -1; }
            else if (i == 6) { df = 2; dc = 0; }
          } else if (sensores.rumbo == 6) { // oeste
            if (i == 1) { df = 1; dc = -1; }
            else if (i == 2) { df = 0; dc = -1; }
            else if (i == 3) { df = -1; dc = -1; }
            else if (i == 6) { df = 0; dc = -2; }
          }
          pos_agente.f = sensores.posF + df;
          pos_agente.c = sensores.posC + dc;
        }
        
        forbidden_cells.insert({pos_agente.f, pos_agente.c});
      }
    }

    queue<NodoBusquedaI> abierta;
    set<NodoBusquedaI> cerrada;

    NodoBusquedaI inicial = {{sensores.posF, sensores.posC, sensores.rumbo}, zaps, {}};
    abierta.push(inicial);

    while (!abierta.empty()) {
      NodoBusquedaI actual = abierta.front();
      abierta.pop();

      if (actual.st.f == sensores.BelPosF && actual.st.c == sensores.BelPosC) {
        plan = actual.camino;
        hayPlan = true;
        break;
      }

      if (cerrada.find(actual) == cerrada.end()) {
        cerrada.insert(actual);

        Action posibles[] = {JUMP, WALK, TURN_SR, TURN_SL};
        for (Action a : posibles) {
          NodoBusquedaI hijo = actual;
          hijo.st = SimularAccionI(actual.st, a);
          hijo.camino.push_back(a);

          bool valido = false;
          if (a == WALK) {
            valido = CasillaTransitableI(hijo.st.f, hijo.st.c, actual.st.f, actual.st.c, actual.tiene_zaps);
            // Verificar que no esté prohibida por otro agente
            if (valido && forbidden_cells.count({hijo.st.f, hijo.st.c}) > 0) valido = false;
          } 
          else if (a == JUMP) {
            ubicacion medio = SimularAccionI(actual.st, WALK);
            bool ok_medio = false;
            if (medio.f >= 0 && medio.f < mapaResultado.size() && medio.c >= 0 && medio.c < mapaResultado[0].size()) {
              char cel_m = mapaResultado[medio.f][medio.c];
              ok_medio = (cel_m != 'P' && cel_m != 'M' && cel_m != 'B');
            }
            bool ok_final = CasillaTransitableI(hijo.st.f, hijo.st.c, actual.st.f, actual.st.c, actual.tiene_zaps);
            valido = ok_medio && ok_final;
            // Verificar que no esté prohibida por otro agente
            if (valido && forbidden_cells.count({hijo.st.f, hijo.st.c}) > 0) valido = false;
          }
          else { 
            valido = true; // Giros
          }

          if (valido) {
            if (mapaResultado[hijo.st.f][hijo.st.c] == 'D') hijo.tiene_zaps = true;

            if (cerrada.find(hijo) == cerrada.end()) {
              abierta.push(hijo);
            }
          }
        }
      }
    }
    if (hayPlan) VisualizaPlan({sensores.posF, sensores.posC, sensores.rumbo}, plan);
  }

  if (hayPlan && !plan.empty()) {
    Action sig = plan.front();
    if ((sig == WALK && sensores.agentes[2] != '_') || 
        (sig == JUMP && (sensores.agentes[2] != '_' || sensores.agentes[6] != '_'))) {
      // Marcar la casilla objetivo como prohibida y replanificar
      ubicacion sig_pos = SimularAccionI({sensores.posF, sensores.posC, sensores.rumbo}, sig);
      forbidden_cells.insert({sig_pos.f, sig_pos.c});
      hayPlan = false;
      plan.clear();
      return IDLE; 
    }
    plan.pop_front();
    return sig;
  }
  return IDLE;
}


/**
 * @brief Comportamiento del ingeniero para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  int pos_tecnico = -1;
  for (int i = 0; i < 16; i++) {
    if (sensores.agentes[i] != '_') { // En el simulador, el otro agente es el Técnico
      pos_tecnico = i;
      break;
    }
  }

  if (pos_tecnico != -1) {
    // Si el Técnico está delante, giramos para despejar el pasillo
    if (pos_tecnico >= 1 && pos_tecnico <= 3) return TURN_SR;

    // Si está en la visión pero no bloquea, intentamos movernos para salir de su zona
    bool obstaculo = (sensores.superficie[2] == 'M' || sensores.superficie[2] == 'P' || sensores.superficie[2] == 'B');
    if (!obstaculo && sensores.agentes[2] == '_') return WALK;
    else return TURN_SL;
  }

  return IDLE; // Si no hay nadie, el Ingeniero se queda quieto para no gastar
}


int HeuristicaTuberias(int f, int c, const vector<pair<int,int>>& plantas) {
    int min_dist = 999999;
    for (const auto& p : plantas) {
        int d = abs(f - p.first) + abs(c - p.second);
        if (d < min_dist) min_dist = d;
    }
    return min_dist;
}


/**
 * @brief Comportamiento del ingeniero para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{

if (!hayPlan) {
    plan.clear();

    auto CosteInstall = [](char t) -> int {
        if (t == 'A') return 50;
        if (t == 'H') return 45;
        if (t == 'S') return 25;
        if (t == 'C' || t == 'U') return 15;
        return 30;
    };

    auto CosteAlterar = [](char t, int op) -> int {
        if (op == 0) return 0;
        if (op == 1) {
            if (t == 'H') return 55;
            if (t == 'S') return 30;
            if (t == 'C' || t == 'U') return 10;
            return 40;
        } else {
            if (t == 'H') return 65;
            if (t == 'S') return 40;
            if (t == 'C' || t == 'U') return 25;
            return 50;
        }
    };

    auto CosteConexion = [&CosteInstall, &CosteAlterar](char terr_prev, char terr_nuevo, int op_nuevo) -> int {
        return CosteInstall(terr_prev) + CosteInstall(terr_nuevo) + CosteAlterar(terr_nuevo, op_nuevo);
    };

    queue<NodoTuberia> abierta;

    // Poda por (f, c, h_tuberia)
    map<EstadoTuberia, int> mejor_eco;  

    int f_ini = sensores.BelPosF, c_ini = sensores.BelPosC;
    int h_ini = mapaCotas[f_ini][c_ini];
    char terr_ini = mapaResultado[f_ini][c_ini];

    for (int op_ini : {0, -1, 1}) {
        if (terr_ini == 'A' && op_ini != 0) continue;
        int h_t_ini = h_ini + op_ini;
        if (h_t_ini < 0 || h_t_ini > 9) continue;

        int eco_ini = CosteAlterar(terr_ini, op_ini);
        if (eco_ini > sensores.max_ecologico) continue;

        EstadoTuberia st_ini = {f_ini, c_ini, h_t_ini};
        auto it = mejor_eco.find(st_ini);
        if (it == mejor_eco.end() || eco_ini < it->second) {
            mejor_eco[st_ini] = eco_ini;
            NodoTuberia n_ini = {st_ini, {{f_ini, c_ini, op_ini}}, eco_ini, 1};
            abierta.push(n_ini);
        }
    }

    int df[] = {-1, 0, 1, 0}, dc[] = {0, 1, 0, -1};

    while (!abierta.empty()) {
        NodoTuberia actual = abierta.front();
        abierta.pop();

        // Poda: descartar si ya encontramos un camino con menos eco a este estado
        if (actual.eco > mejor_eco[actual.st]) continue;

        // META
        if (mapaResultado[actual.st.f][actual.st.c] == 'U') {
            cout << "PLAN ENCONTRADO. Longitud: " << actual.longitud
                 << " | Impacto ecologico: " << actual.eco << endl;
            hayPlan = true;
            VisualizaRedTuberias(actual.camino);
            break;
        }

        char terr_actual = mapaResultado[actual.st.f][actual.st.c];

        for (int i = 0; i < 4; i++) {
            int nf = actual.st.f + df[i], nc = actual.st.c + dc[i];
            if (nf < 0 || nf >= (int)mapaResultado.size() ||
                nc < 0 || nc >= (int)mapaResultado[0].size()) continue;

            char terr = mapaResultado[nf][nc];
            if (terr == 'M' || terr == 'P' || terr == 'B') continue;

            int h_suelo_hijo = mapaCotas[nf][nc];

            for (int nh : {actual.st.h_tuberia, actual.st.h_tuberia - 1}) {
                if (nh < 0 || nh > 9) continue;

                int op_hijo = nh - h_suelo_hijo;
                if (op_hijo < -1 || op_hijo > 1) continue;
                if (terr == 'A' && op_hijo != 0) continue;

                int eco_nuevo = actual.eco + CosteConexion(terr_actual, terr, op_hijo);
                if (eco_nuevo > sensores.max_ecologico) continue;

                int long_nueva = actual.longitud + 1;
                EstadoTuberia st_hijo = {nf, nc, nh};

                auto it = mejor_eco.find(st_hijo);
                if (it == mejor_eco.end() || eco_nuevo < it->second) {
                    mejor_eco[st_hijo] = eco_nuevo;
                    NodoTuberia hijo = actual;
                    hijo.st = st_hijo;
                    hijo.longitud = long_nueva;
                    hijo.eco = eco_nuevo;
                    hijo.camino.push_back({nf, nc, op_hijo});
                    abierta.push(hijo);
                }
            }
        }
    }
}

return IDLE;

}

/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 * Planifica red de tuberías y la construye junto al Técnico.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores)
{
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D') {
    zaps = true;
  }

  // Calcular ruta óptima y guardarla en g_plan_n5
  if (g_plan_n5.empty()) {

    auto CosteInstall = [](char t) -> int {
        if (t == 'A') return 50;
        if (t == 'H') return 45;
        if (t == 'S') return 25;
        if (t == 'C' || t == 'U') return 15;
        return 30;
    };

    auto CosteAlterar = [](char t, int op) -> int {
        if (op == 0) return 0;
        if (op == 1) {
            if (t == 'H') return 55;
            if (t == 'S') return 30;
            if (t == 'C' || t == 'U') return 10;
            return 40;
        } else {
            if (t == 'H') return 65;
            if (t == 'S') return 40;
            if (t == 'C' || t == 'U') return 25;
            return 50;
        }
    };

    auto CosteConexion = [&](char terr_prev, char terr_nuevo, int op_nuevo) -> int {
        return CosteInstall(terr_prev) + CosteInstall(terr_nuevo) + CosteAlterar(terr_nuevo, op_nuevo);
    };

    queue<NodoTuberia> abierta;
    map<EstadoTuberia, int> mejor;

    int f_ini = sensores.BelPosF, c_ini = sensores.BelPosC;
    int h_ini = mapaCotas[f_ini][c_ini];
    char terr_ini = mapaResultado[f_ini][c_ini];

    for (int op_ini : {0, -1, 1}) {
        if (terr_ini == 'A' && op_ini != 0) continue;
        int h_t_ini = h_ini + op_ini;
        if (h_t_ini < 0 || h_t_ini > 9) continue;

        int eco_ini = CosteAlterar(terr_ini, op_ini);
        if (eco_ini > sensores.max_ecologico) continue;

        EstadoTuberia st_ini = {f_ini, c_ini, h_t_ini};
        auto it = mejor.find(st_ini);
        if (it == mejor.end() || eco_ini < it->second) {
            mejor[st_ini] = eco_ini;
            NodoTuberia n_ini = {st_ini, {{f_ini, c_ini, op_ini}}, eco_ini, 1, op_ini};
            abierta.push(n_ini);
        }
    }

    int df[] = {-1, 0, 1, 0}, dc[] = {0, 1, 0, -1};

    while (!abierta.empty()) {
        NodoTuberia actual = abierta.front();
        abierta.pop();

        if (actual.eco > mejor[actual.st]) continue;

        // La meta, si llegamos a 'U'
        if (mapaResultado[actual.st.f][actual.st.c] == 'U') {
            VisualizaRedTuberias(actual.camino);

            // Guardar ruta en g_plan_n5 
            for (auto& paso : actual.camino) {
                g_plan_n5.push_back(paso);
            }
            break;  
        }

        char terr_actual = mapaResultado[actual.st.f][actual.st.c];

        for (int i = 0; i < 4; i++) {
            int nf = actual.st.f + df[i], nc = actual.st.c + dc[i];
            if (nf < 0 || nf >= (int)mapaResultado.size() ||
                nc < 0 || nc >= (int)mapaResultado[0].size()) continue;

            char terr = mapaResultado[nf][nc];
            if (terr == 'M' || terr == 'P' || terr == 'B') continue;

            int h_suelo_hijo = mapaCotas[nf][nc];

            for (int nh : {actual.st.h_tuberia, actual.st.h_tuberia - 1}) {
                if (nh < 0 || nh > 9) continue;

                int op_hijo = nh - h_suelo_hijo;
                if (op_hijo < -1 || op_hijo > 1) continue;
                if (terr == 'A' && op_hijo != 0) continue;

                int eco_nuevo = actual.eco + CosteConexion(terr_actual, terr, op_hijo);
                if (eco_nuevo > sensores.max_ecologico) continue;

                int long_nueva = actual.longitud + 1;
                EstadoTuberia st_hijo = {nf, nc, nh};

                auto it = mejor.find(st_hijo);
                if (it == mejor.end() || eco_nuevo < it->second) {
                    mejor[st_hijo] = eco_nuevo;
                    NodoTuberia hijo = actual;
                    hijo.st = st_hijo;
                    hijo.longitud = long_nueva;
                    hijo.eco = eco_nuevo;
                    hijo.camino.push_back({nf, nc, op_hijo});
                    abierta.push(hijo);
                }
            }
        }
    }
  }

  // Una vez calculada la ruta, ir a la Belkanita usando navegación Nivel 2
  if (g_plan_n5.empty()) return IDLE;

  if (sensores.posF == sensores.BelPosF && sensores.posC == sensores.BelPosC) {
    enPos = true;
  }

  if (!enPos) {
    return ComportamientoIngenieroNivel_2(sensores);
  }

  if(g_plan_n5[0].op == 1){
    g_plan_n5[0].op = 0;
    return RAISE;
  }
  else  if(g_plan_n5[0].op == -1){
    g_plan_n5[0].op = 0;
    return DIG;
  }

   // Ya en Belkanita: ejecutar protocolo de construccion
  if (g_plan_n5.size() < 2) return IDLE;



  // Orientarse hacia la siguiente casilla
  if (fase_n5 == 0) {
    sig_f = g_plan_n5[contRuta].fil;
    sig_c = g_plan_n5[contRuta].col;
    sig_op = g_plan_n5[contRuta].op;

    int df = sig_f - sensores.posF;
    int dc = sig_c - sensores.posC;

    Orientacion deseada = norte;
    if (df == -1 && dc == 0) deseada = norte;
    else if (df == -1 && dc == 1) deseada = noreste;
    else if (df == 0 && dc == 1) deseada = este;
    else if (df == 1 && dc == 1) deseada = sureste;
    else if (df == 1 && dc == 0) deseada = sur;
    else if (df == 1 && dc == -1) deseada = suroeste;
    else if (df == 0 && dc == -1) deseada = oeste;
    else if (df == -1 && dc == -1) deseada = noroeste;

    int diff = (deseada - sensores.rumbo + 8) % 8;
    if (diff == 0) {
      fase_n5 = 1;
      contRuta++;
      return IDLE;
    } else if (diff <= 4) {
      return TURN_SR;
    } else {
      return TURN_SL;
    }
  }

  // Estado 1: Hacer COME para que el tecnico reciba la posicion
  if (fase_n5 == 1) {
    fase_n5 = 2;
    cout << "Ing hace COME";
    return COME;
  }

  // Estado 2: Avanzar a la siguiente casilla
  if (fase_n5 == 2) {
    if (sensores.posF == sig_f && sensores.posC == sig_c) {
      fase_n5 = 3;
      giros_180_n5 = 0;
      return IDLE;
    }
    return WALK;
  }

  // Estado 3: Girar 180 grados (4 giros)
  if (fase_n5 == 3) {
    if (giros_180_n5 < 4) {
      giros_180_n5++;
      return TURN_SR;
    } else {
      fase_n5 = -1;
      return IDLE;
    }
  }

  // Install inicial  
  if (fase_n5 == -1) {
    if (sig_op == 1){
      sig_op = 0;
      return RAISE;
    } 
    if (sig_op == -1){
      sig_op = 0;
      return DIG;
    } 
    if (sensores.agentes[2] == 't') {
      if (g_tecnico_listo_install) {
        g_ingeniero_listo_install = true;
        current_tramo_n5 = 1;  
        fase_n5 = 0;  
        return INSTALL;
      }
    }
    return IDLE;  // Esperar al técnico
  }

  
  int idx_actual = current_tramo_n5;  
  if (idx_actual >= (int)g_plan_n5.size()) return IDLE;

  // Install (cara a cara con técnico)
  if (fase_n5 == 4) {
    if (sig_op == 1){
      sig_op = 0;
      return RAISE;
    } 
    if (sig_op == -1){
      sig_op = 0;
      return DIG;
    } 
    if (g_tecnico_listo_install) {
      // Ambos listos: hacer INSTALL y resetear flags
      g_ingeniero_listo_install = true;
      current_tramo_n5++;
      if (current_tramo_n5 >= (int)g_plan_n5.size() - 1) return IDLE;
      fase_n5 = 0;
      return INSTALL;
    }
    return IDLE; // Esperar al técnico
  }

  // Posición siguiente
  int sig_idx = current_tramo_n5 + 1;
  if (sig_idx >= (int)g_plan_n5.size()) return IDLE;
  sig_f = g_plan_n5[sig_idx].fil;
  sig_c = g_plan_n5[sig_idx].col;

  // FASE 0: Orientarse hacia la siguiente casilla 
  if (fase_n5 == 0) {
    int df = sig_f - sensores.posF;
    int dc = sig_c - sensores.posC;

    Orientacion deseada = norte;
    if (df == -1 && dc == 0) deseada = norte;
    else if (df == -1 && dc == 1) deseada = noreste;
    else if (df == 0 && dc == 1) deseada = este;
    else if (df == 1 && dc == 1) deseada = sureste;
    else if (df == 1 && dc == 0) deseada = sur;
    else if (df == 1 && dc == -1) deseada = suroeste;
    else if (df == 0 && dc == -1) deseada = oeste;
    else if (df == -1 && dc == -1) deseada = noroeste;

    int diff = (deseada - sensores.rumbo + 8) % 8;
    if (diff == 0) {
      fase_n5 = 1;
      return IDLE;
    } else if (diff <= 4) {
      return TURN_SR;
    } else {
      return TURN_SL;
    }
  }

  // Hacer COME (ingeniero SIGUE en posición actual, técnico vendrá aquí)
  if (fase_n5 == 1) {
    fase_n5 = 2;
    return COME;
  }

  // Avanzar a la siguiente casilla 
  if (fase_n5 == 2) {
    if (sensores.posF == sig_f && sensores.posC == sig_c) {
      fase_n5 = 3;
      giros_180_n5 = 0;
      return IDLE;
    }
    return WALK;
  }

  // FASE 3: Girar 180 grados (4 giros) para mirar hacia atrás (posición idx_actual)
  if (fase_n5 == 3) {
    if (giros_180_n5 < 4) {
      giros_180_n5++;
      return TURN_SR;
    } else {
      fase_n5 = 4;  // Ahora esperar al técnico para INSTALL
      return IDLE;
    }
  }

  return IDLE;
  }

  Action ComportamientoIngeniero::InvestigacionInteligenteI(Sensores sensores) {

    if (!plan.empty()) {
        Action sig = plan.front();
        ubicacion pos_actual = {sensores.posF, sensores.posC, sensores.rumbo};
        bool seguro = true;
        
        if (sig == WALK) {
            ubicacion sig_pos = Delante(pos_actual);
            if (!CasillaTransitableI(sig_pos.f, sig_pos.c, pos_actual.f, pos_actual.c, zaps)) {
                seguro = false;
            }
        } 
        else if (sig == JUMP) {
             ubicacion medio = Delante(pos_actual);
             ubicacion final = Delante(medio);
             
             // Comprobar que no chocamos en el medio
             if (medio.f >= 0 && medio.f < mapaResultado.size() && medio.c >= 0 && medio.c < mapaResultado[0].size()) {
                 char c_medio = mapaResultado[medio.f][medio.c];
                 if (c_medio == 'P' || c_medio == 'M' || c_medio == 'B' || c_medio == '?') seguro = false;
             } else {
                 seguro = false;
             }
             // Comprobar que el aterrizaje es transitable
             if (seguro && !CasillaTransitableI(final.f, final.c, pos_actual.f, pos_actual.c, zaps)) {
                 seguro = false;
             }
        }
        
        if (seguro) {
            plan.pop_front();
            return sig;
        } else {
            plan.clear(); // Plan peligroso, lo borramos para replanificar
        }
    }

    // BFS para buscar la frontera con la zona inexplorada ('?')
    struct Nodo {
        ubicacion loc;
        list<Action> camino;
    };

    queue<Nodo> q;
    vector<vector<vector<bool>>> visitados(
        mapaResultado.size(), 
        vector<vector<bool>>(mapaResultado[0].size(), vector<bool>(8, false))
    );

    ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
    q.push({inicial, {}});
    visitados[inicial.f][inicial.c][inicial.brujula] = true;

    while (!q.empty()) {
        Nodo actual = q.front();
        q.pop();

        // Meta: encontrar un '?'
        bool ve_desconocido = false;
        int df[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
        
        for (int i = 0; i < 8; i++) {
            int nf = actual.loc.f + df[i];
            int nc = actual.loc.c + dc[i];
            if (nf >= 0 && nf < mapaResultado.size() && nc >= 0 && nc < mapaResultado[0].size()) {
                if (mapaResultado[nf][nc] == '?') {
                    ve_desconocido = true;
                    break;
                }
            }
        }

        // Si ve una zona inexplorada, ejecutamos la primera acción del plan para acercarnos
        if (ve_desconocido && !actual.camino.empty()) {
            plan = actual.camino;
            Action primera = plan.front();
            plan.pop_front();
            return primera;
        }

        vector<Action> posibles = {WALK, TURN_SL, TURN_SR, JUMP};

        for (Action act : posibles) {
            ubicacion siguiente = actual.loc;
            bool movimiento_seguro = false;

            if (act == WALK) {
                siguiente = Delante(actual.loc);
                // Usamos la función CasillaTransitableI que comprueba alturas y precipicios
                if (CasillaTransitableI(siguiente.f, siguiente.c, actual.loc.f, actual.loc.c, zaps) && sensores.superficie[2] != 'A') {
                    movimiento_seguro = true;
                }
            } 
            else if (act == JUMP) {
                ubicacion medio = Delante(actual.loc);
                ubicacion final = Delante(medio);

                if (medio.f >= 0 && medio.f < mapaResultado.size() && medio.c >= 0 && medio.c < mapaResultado[0].size() && sensores.superficie[6] != 'A') {
                    char c_medio = mapaResultado[medio.f][medio.c];
                    
                    // La casilla sobre la que saltamos no debe ser muro, precipicio, bosque ni desconocida
                    if (c_medio != 'P' && c_medio != 'M' && c_medio != 'B' && c_medio != '?') {
                        // El aterrizaje se verifica con alturas y límites
                        if (CasillaTransitableI(final.f, final.c, actual.loc.f, actual.loc.c, zaps)) {
                            siguiente = final;
                            movimiento_seguro = true;
                        }
                    }
                }
            } 
            else { // Giros
                if (act == TURN_SL) siguiente.brujula = (Orientacion)(((int)siguiente.brujula + 7) % 8);
                else siguiente.brujula = (Orientacion)(((int)siguiente.brujula + 1) % 8);
                movimiento_seguro = true;
            }

            if (movimiento_seguro && !visitados[siguiente.f][siguiente.c][siguiente.brujula]) {
                visitados[siguiente.f][siguiente.c][siguiente.brujula] = true;
                list<Action> nuevo_camino = actual.camino;
                nuevo_camino.push_back(act);
                q.push({siguiente, nuevo_camino});
            }
        }
    }

    // Si no encontramos '?', avanzamos con nivel 1
    return ComportamientoIngenieroNivel_1(sensores);
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 6.
 * Planifica red de tuberías y la construye junto al Técnico (sin conocer el mapa previamente).
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores) {
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D') {
    zaps = true;
  }

  // Planificacion en zonas descubiertas
  if (g_plan_n5.empty()) {
    auto CosteInstall = [](char t) -> int {
        if (t == 'A') return 50; if (t == 'H') return 45;
        if (t == 'S') return 25; if (t == 'C' || t == 'U') return 15;
        return 30;
    };
    auto CosteAlterar = [](char t, int op) -> int {
        if (op == 0) return 0;
        if (op == 1) {
            if (t == 'H') return 55; if (t == 'S') return 30;
            if (t == 'C' || t == 'U') return 10; return 40;
        } else {
            if (t == 'H') return 65; if (t == 'S') return 40;
            if (t == 'C' || t == 'U') return 25; return 50;
        }
    };
    auto CosteConexion = [&](char terr_prev, char terr_nuevo, int op_nuevo) -> int {
        return CosteInstall(terr_prev) + CosteInstall(terr_nuevo) + CosteAlterar(terr_nuevo, op_nuevo);
    };

    queue<NodoTuberia> abierta;
    map<EstadoTuberia, int> mejor;

    int f_ini = sensores.BelPosF, c_ini = sensores.BelPosC;
    
    // Asegurarnos de que sabemos dónde está la Belkanita antes de planificar
    if (mapaResultado[f_ini][c_ini] != '?') {
        int h_ini = mapaCotas[f_ini][c_ini];
        char terr_ini = mapaResultado[f_ini][c_ini];

        for (int op_ini : {0, -1, 1}) {
            if (terr_ini == 'A' && op_ini != 0) continue;
            int h_t_ini = h_ini + op_ini;
            if (h_t_ini < 0 || h_t_ini > 9) continue;

            int eco_ini = CosteAlterar(terr_ini, op_ini);
            if (eco_ini > sensores.max_ecologico) continue;

            EstadoTuberia st_ini = {f_ini, c_ini, h_t_ini};
            auto it = mejor.find(st_ini);
            if (it == mejor.end() || eco_ini < it->second) {
                mejor[st_ini] = eco_ini;
                NodoTuberia n_ini = {st_ini, {{f_ini, c_ini, op_ini}}, eco_ini, 1, op_ini};
                abierta.push(n_ini);
            }
        }
    }

    int df[] = {-1, 0, 1, 0}, dc[] = {0, 1, 0, -1};

    while (!abierta.empty()) {
        NodoTuberia actual = abierta.front();
        abierta.pop();

        if (actual.eco > mejor[actual.st]) continue;

        if (mapaResultado[actual.st.f][actual.st.c] == 'U') {
            VisualizaRedTuberias(actual.camino);
            for (auto& paso : actual.camino) g_plan_n5.push_back(paso);
            break; 
        }

        char terr_actual = mapaResultado[actual.st.f][actual.st.c];

        for (int i = 0; i < 4; i++) {
            int nf = actual.st.f + df[i], nc = actual.st.c + dc[i];
            if (nf < 0 || nf >= (int)mapaResultado.size() ||
                nc < 0 || nc >= (int)mapaResultado[0].size()) continue;

            char terr = mapaResultado[nf][nc];
            // CLAVE: Evitamos zonas desconocidas '?' además de obstáculos
            if (terr == 'M' || terr == 'P' || terr == 'B' || terr == '?') continue;

            int h_suelo_hijo = mapaCotas[nf][nc];

            for (int nh : {actual.st.h_tuberia, actual.st.h_tuberia - 1}) {
                if (nh < 0 || nh > 9) continue;

                int op_hijo = nh - h_suelo_hijo;
                if (op_hijo < -1 || op_hijo > 1) continue;
                if (terr == 'A' && op_hijo != 0) continue;

                int eco_nuevo = actual.eco + CosteConexion(terr_actual, terr, op_hijo);
                if (eco_nuevo > sensores.max_ecologico) continue;

                int long_nueva = actual.longitud + 1;
                EstadoTuberia st_hijo = {nf, nc, nh};

                auto it = mejor.find(st_hijo);
                if (it == mejor.end() || eco_nuevo < it->second) {
                    mejor[st_hijo] = eco_nuevo;
                    NodoTuberia hijo = actual;
                    hijo.st = st_hijo;
                    hijo.longitud = long_nueva;
                    hijo.eco = eco_nuevo;
                    hijo.camino.push_back({nf, nc, op_hijo});
                    abierta.push(hijo);
                }
            }
        }
    }

    // Si después de todo esto el plan sigue vacío, no hay mapa suficiente: Exploramos
    if (g_plan_n5.empty()) {
        return InvestigacionInteligenteI(sensores);
    }
  }

  // Vamos a la belkanita
  if (sensores.posF == sensores.BelPosF && sensores.posC == sensores.BelPosC) {
    enPos = true;
  }

  if (!enPos) {
    Action act = ComportamientoIngenieroNivel_2(sensores);
    // Si el camino a la belkanita esta cortado por casiilas '?'
    if (act == IDLE && !hayPlan) {
        g_plan_n5.clear(); // Descartamos la tubería porque no podemos llegar
        return ComportamientoIngenieroNivel_1(sensores); // Seguimos explorando
    }
    return act;
  }

  // Construccion
  if(g_plan_n5[0].op == 1){
    g_plan_n5[0].op = 0;
    return RAISE;
  }
  else if(g_plan_n5[0].op == -1){
    g_plan_n5[0].op = 0;
    return DIG;
  }

  if (g_plan_n5.size() < 2) return IDLE;

  if (fase_n5 == 0) {
    sig_f = g_plan_n5[contRuta].fil;
    sig_c = g_plan_n5[contRuta].col;
    sig_op = g_plan_n5[contRuta].op;

    int df = sig_f - sensores.posF;
    int dc = sig_c - sensores.posC;

    Orientacion deseada = norte;
    if (df == -1 && dc == 0) deseada = norte;
    else if (df == -1 && dc == 1) deseada = noreste;
    else if (df == 0 && dc == 1) deseada = este;
    else if (df == 1 && dc == 1) deseada = sureste;
    else if (df == 1 && dc == 0) deseada = sur;
    else if (df == 1 && dc == -1) deseada = suroeste;
    else if (df == 0 && dc == -1) deseada = oeste;
    else if (df == -1 && dc == -1) deseada = noroeste;

    int diff = (deseada - sensores.rumbo + 8) % 8;
    if (diff == 0) {
      fase_n5 = 1;
      contRuta++;
      return IDLE;
    } else if (diff <= 4) { return TURN_SR; } 
    else { return TURN_SL; }
  }

  if (fase_n5 == 1) {
    fase_n5 = 2;
    return COME;
  }

  if (fase_n5 == 2) {
    if (sensores.posF == sig_f && sensores.posC == sig_c) {
      fase_n5 = 3;
      giros_180_n5 = 0;
      return IDLE;
    }
    return WALK;
  }

  if (fase_n5 == 3) {
    if (giros_180_n5 < 4) {
      giros_180_n5++;
      return TURN_SR;
    } else {
      fase_n5 = -1;
      return IDLE;
    }
  }

  if (fase_n5 == -1) {
    if (sig_op == 1){ sig_op = 0; return RAISE; } 
    if (sig_op == -1){ sig_op = 0; return DIG; } 
    
    if (sensores.agentes[2] == 't') {
      if (g_tecnico_listo_install) {
        g_ingeniero_listo_install = true;
        current_tramo_n5 = 1;  
        fase_n5 = 0; 
        return INSTALL;
      }
    }
    return IDLE;  
  }

  int idx_actual = current_tramo_n5; 
  if (idx_actual >= (int)g_plan_n5.size()) return IDLE;

  if (fase_n5 == 4) {
    if (sig_op == 1){ sig_op = 0; return RAISE; } 
    if (sig_op == -1){ sig_op = 0; return DIG; } 
    
    if (g_tecnico_listo_install) {
      g_ingeniero_listo_install = true;
      current_tramo_n5++;
      if (current_tramo_n5 >= (int)g_plan_n5.size() - 1) return IDLE;
      fase_n5 = 0;
      return INSTALL;
    }
    return IDLE;
  }

  int sig_idx = current_tramo_n5 + 1;
  if (sig_idx >= (int)g_plan_n5.size()) return IDLE;
  sig_f = g_plan_n5[sig_idx].fil;
  sig_c = g_plan_n5[sig_idx].col;

  if (fase_n5 == 0) {
    int df = sig_f - sensores.posF;
    int dc = sig_c - sensores.posC;

    Orientacion deseada = norte;
    if (df == -1 && dc == 0) deseada = norte;
    else if (df == -1 && dc == 1) deseada = noreste;
    else if (df == 0 && dc == 1) deseada = este;
    else if (df == 1 && dc == 1) deseada = sureste;
    else if (df == 1 && dc == 0) deseada = sur;
    else if (df == 1 && dc == -1) deseada = suroeste;
    else if (df == 0 && dc == -1) deseada = oeste;
    else if (df == -1 && dc == -1) deseada = noroeste;

    int diff = (deseada - sensores.rumbo + 8) % 8;
    if (diff == 0) {
      fase_n5 = 1;
      return IDLE;
    } else if (diff <= 4) { return TURN_SR; } 
    else { return TURN_SL; }
  }

  if (fase_n5 == 1) {
    fase_n5 = 2;
    return COME;
  }

  if (fase_n5 == 2) {
    if (sensores.posF == sig_f && sensores.posC == sig_c) {
      fase_n5 = 3;
      giros_180_n5 = 0;
      return IDLE;
    }
    return WALK;
  }

  if (fase_n5 == 3) {
    if (giros_180_n5 < 4) {
      giros_180_n5++;
      return TURN_SR;
    } else {
      fase_n5 = 4;  
      return IDLE;
    }
  }

  return IDLE;
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la informaciÃ³n de los sensores.
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
 * Para el ingeniero: desnivel mÃ¡ximo 1 sin zapatillas, 2 con zapatillas.
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
 * @brief Devuelve la posiciÃ³n (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal segÃºn la orientaciÃ³n actual (8 direcciones).
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
 * @brief Imprime las coordenadas y operaciones de un plan de tuberÃ­a.
 *
 * @param plan  Lista de pasos (fila, columna, operaciÃ³n),
 *              donde operacion = -1 (DIG), operaciÃ³n = 1 (RAISE).
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
 *        su visualizaciÃ³n en el mapa 2D.
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
 * @brief Convierte un plan de tuberÃ­a en la lista de casillas usada
 *        por el sistema de visualizaciÃ³n.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tuberÃ­a.
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
#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <list>

extern bool g_ingeniero_listo_install;
extern bool g_tecnico_listo_install;
extern vector<Paso> g_plan_n5;

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores) {
  Action accion = IDLE;


  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_2(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
}

int ComportamientoTecnico::veoCasillaInteresanteT0(char i, char c, char d, bool zaps){

  if (c == 'U') return 2;
  else if (d == 'U') return 3;
  else if (i == 'U') return 1;
  // Para cuando las zapatillas tengan importancia
  /*else if (!zaps){
    if (c == 'D') return 2;
    else if (d == 'D') return 3;
    else if (i == 'D') return 1;
  } */
  
  if (c == 'C'||c == 'D') return 2;
  else if (d == 'C'||c == 'D') return 3;
  else if (i == 'C'||c == 'D') return 1;
  
  return 0;
}

int ComportamientoTecnico::veoCasillaInteresanteT1(char i, char c, char d, bool zaps){
  // En el caso del tecnico he preferido ser mas conservador y no dejar que entre al agua ni a la hierba,
  // para hacer un equilibrio entre energía y mapa investigado.
  if (!zaps){
    if (c == 'D') return 2;
    else if (d == 'D') return 3;
    else if (i == 'D') return 1;
  } 
  if (c == 'C') return 2;
  else if (d == 'C') return 3;
  else if (i == 'C') return 1;
  else if (c == 'U') return 2;
  else if (d == 'U') return 3;
  else if (i == 'U') return 1;
  else if (c == 'S') return 2;
  else if (d == 'S') return 3;
  else if (i == 'S') return 1;
  else if (c == 'H') return 2;
  else if (d == 'H') return 3;
  else if (i == 'H') return 1;

  return 0;
}

char ComportamientoTecnico::viablePorAlturaT(char casilla, int dif){
  if (abs(dif) <= 1) return casilla;
  else return 'P';
}


// Niveles del técnico
Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores) {

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

  // Detecta si hay bloqueo y gira, mientras activa el bool de bloqueado
  if (last_action == WALK &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Vemos qué opciones tienen altura correcta
  char i = viablePorAlturaT(sensores.superficie[1],
                           sensores.cota[1] - sensores.cota[0]);

  char c = viablePorAlturaT(sensores.superficie[2],
                           sensores.cota[2] - sensores.cota[0]);

  char d = viablePorAlturaT(sensores.superficie[3],
                           sensores.cota[3] - sensores.cota[0]);
                           
  // Nos quedamos con la posicion de la casilla que más nos interese
  int pos = veoCasillaInteresanteT0(i, c, d, zaps);
  
  // Si estamos en U, solo avanzamos en caso de tener otra U en un paso, sino nos mantenemos
  if (sensores.superficie[0] == 'U') {
    if (c == 'U') return WALK;
    else if (d == 'U') return TURN_SR;
    else if (i == 'U') return TURN_SL;
    else{
      return IDLE;
    } 
  }

  // Tratamos el bloqueo (creado para colisiones con ingeniero en U)
  if (en_bloqueo && c == 'U'){
    en_bloqueo = false;
    en_bloqueo_U = true;
    contador_giros++;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Tratamos el bloqueo con el ingeniero
  if (en_bloqueo_U){
    if (pos == 2){
      en_bloqueo_U = false;
      accion = WALK;
    } 
    else{
      contador_giros++;
      accion = giro_preferido;
    } 
    last_action = accion;
    return accion;
  }
  
  // Llegados a este caso, era un bloqueo con un muro o ingeniero
  if (en_bloqueo) {

    if (pos == 2){
      en_bloqueo = false;
      accion = WALK;
    } 
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
  // Si llegamos aquí, no hay bloqueos
  else {
    // Si hay una U vamos a por ella
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
    // En caso de estar en un bucle, esta es una forma de usar la memoria para salir de dicho bucle por otro camino
    else if (es_camino(sensores.superficie[1]) && (i != 'P') && visitas[pos_izq.f][pos_izq.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SL;
    }
    else if (es_camino(sensores.superficie[3]) && (d != 'P') && visitas[pos_der.f][pos_der.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SR;
    }
    // Llegados aquí simplemente tomamos la posicion que más nos hubiese interesasdo antes (1-i, 2-c, 3-d, 0-ninguna)
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
    // Llegados aquí, estamos sin salidas, luego tenemos que girar para buscar más caminos
    else {
      // Esto es para que primero gire para un lado, y si no hay salidas significa que por ese lado no había camino, y en ese punto giramos 
      // por completo hacia el otro por si habia un camino justo en ese lado. Sino hubiese, terminaría de dar la vuelta porque la salida estabaa detras
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
bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

bool ComportamientoTecnico::es_camino1(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U' || c == 'S' || c == 'H');
}


/**
 * @brief Comportamiento reactivo del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores) {
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

  // Detecta si hay bloqueo
  if (last_action == WALK &&
      sensores.posF == last_f &&
      sensores.posC == last_c) {
    en_bloqueo = true;
    last_action = giro_preferido;
    return giro_preferido;
  }

  // Comprobamos alturas
  char i = viablePorAlturaT(sensores.superficie[1],
                           sensores.cota[1] - sensores.cota[0]);

  char c = viablePorAlturaT(sensores.superficie[2],
                           sensores.cota[2] - sensores.cota[0]);

  char d = viablePorAlturaT(sensores.superficie[3],
                           sensores.cota[3] - sensores.cota[0]);
                           
  int pos = veoCasillaInteresanteT1(i, c, d, zaps);

  // Tratamos bloqueo
  if (en_bloqueo) {
    if (sensores.superficie[2] != 'D' && es_camino1(sensores.superficie[2]) && sensores.superficie[2] == c){
      en_bloqueo = false;
      accion = WALK;
    } 
    else accion = giro_preferido;
    last_action = accion;
    return accion;
  }
   // Aquí ya no hay bloqueo, priorizamos zonas menos visitadas (aunque le permita entrar al agua, no quiero que sea por ser menos visitado).
  else {
    if (es_camino1(sensores.superficie[1]) && (i != 'A') && (i != 'P') && visitas[pos_izq.f][pos_izq.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SL;
    }
    else if (es_camino1(sensores.superficie[3]) && (i != 'A') && (d != 'P') && visitas[pos_der.f][pos_der.c] < visitas[pos_frente.f][pos_frente.c]) {
      accion = TURN_SR;
    }
     else if (es_camino1(sensores.superficie[2]) && (i != 'A') && (c != 'P') && sensores.superficie[2] == c) {
      accion = WALK;
    }
    // Llegados a este caso, optamos por la posicion anteriormente escogida
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
    // Estamos sin salida, luego tenemos que girar hasta poder avanzar de nuevo
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

    // Sirve para variar los giros, y permite romper ciertos ciclos
    if (contador_giros >= 15) {
      giro_preferido = (giro_preferido == TURN_SL) ? TURN_SR : TURN_SL;
      contador_giros = 0;
    }
  }

  // Es otra forma de romper ciclos
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

/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {

  if (sensores.superficie[0] == 'D') {
    zaps = true;
  }

  // Localizar la posición exacta del Ingeniero en el cono de visión
  int pos_ingeniero = -1;
  for (int i = 0; i < 16; i++) {
    if (sensores.agentes[i] != '_') {
      pos_ingeniero = i;
      break;
    }
  }

  // Lógica si el Ingeniero está cerca
  if (pos_ingeniero != -1) {
    // Si el ingeniero está en nuestro "carril central" (casillas 1, 2, 3)
    // Nos estamos bloqueando mutuamente. ¡GIRAR para cambiar de trayectoria!
    if (pos_ingeniero >= 1 && pos_ingeniero <= 3) {
      return TURN_SR; 
    }

    // Si el ingeniero está a la vista pero no bloqueando el frente inmediato,
    // intentamos avanzar para liberar la zona actual.
    bool obstaculo = (sensores.superficie[2] == 'M' || sensores.superficie[2] == 'P' || sensores.superficie[2] == 'B');
    int dif_altura = abs((int)sensores.cota[0] - (int)sensores.cota[2]);

    // Solo caminamos si es seguro y no hay nadie en la casilla de destino
    if (!obstaculo && dif_altura <= 1 && sensores.agentes[2] == '_') {
      return WALK;
    } else {
      // Si no podemos huir hacia adelante, giramos rápido
      return TURN_SR;
    }
  }

  // Rotamos para ver si viene el ingeniero
  return TURN_SR;
}

// HEURÍSTICA: Distancia de Chebyshev  
int HeuristicaT(int f, int c, int dF, int dC) {
  return max(abs(f - dF), abs(c - dC));
}

// Costes de energia
int GastoWALK(char casillaOrigen, int hOrigen, int hDestino) {
    int base = 1;
    bool aplicaAltura = false;

    if (casillaOrigen == 'A') {
        base = 60;
        aplicaAltura = true;
    } else if (casillaOrigen == 'H') {
        base = 6;
        aplicaAltura = true;
    } else if (casillaOrigen == 'S') {
        base = 3;
        aplicaAltura = true;
    }

    // Solo sumamos/restamos si la casilla de ORIGEN es A, H o S
    if (aplicaAltura) {
        if (hDestino > hOrigen) return base + 5;
        if (hDestino < hOrigen) return base - 2;
    }
    
    return base; // Para el resto devuelve base
}

// Función para el coste de giros
int GastoGiro(char casillaOrigen) {
    if (casillaOrigen == 'A') return 5;
    if (casillaOrigen == 'H') return 2;
    if (casillaOrigen == 'S') return 1;
    return 1; // Resto de casillas
}

ubicacion ComportamientoTecnico::SimularAccionT(ubicacion actual, Action a) {
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

bool ComportamientoTecnico::CasillaTransitableT(int f, int c, int f_ant, int c_ant, bool con_zaps) {
  // Límites del mapa
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) return false;

  // Obstáculos
  char celda = mapaResultado[f][c];
  if (celda == 'P' || celda == 'M' || celda == '?') return false;
  if (celda == 'B' && !con_zaps) return false;

  // Control altura
  int h_destino = (int)mapaCotas[f][c];
  int h_origen = (int)mapaCotas[f_ant][c_ant];
  int dif = abs(h_destino - h_origen);

  return (dif <= 1); 
}

/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  if (sensores.superficie[0] == 'D') zaps = true;

    if (!hayPlan) {
        plan.clear();
        priority_queue<NodoBusquedaT, vector<NodoBusquedaT>, greater<NodoBusquedaT>> abierta;
        // Mapa para control de nodos cerrados: <Estado, Energía Mínima>
        map<pair<ubicacion, bool>, int, ComparaEstado> cerrados;

        ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
        NodoBusquedaT n_inicial = {inicial, zaps, {}, 0, 0};
        n_inicial.f = HeuristicaT(inicial.f, inicial.c, sensores.BelPosF, sensores.BelPosC);
        abierta.push(n_inicial);

        while (!abierta.empty()) {
            NodoBusquedaT actual = abierta.top();
            abierta.pop();

            if (actual.st.f == sensores.BelPosF && actual.st.c == sensores.BelPosC) {
                plan = actual.camino;
                hayPlan = true;
                break;
            }

            pair<ubicacion, bool> estado_actual = {actual.st, actual.tiene_zaps};
            if (cerrados.count(estado_actual) && cerrados[estado_actual] <= actual.g) continue;
            cerrados[estado_actual] = actual.g;

            Action acciones[] = {WALK, TURN_SR, TURN_SL};
            for (Action a : acciones) {
                NodoBusquedaT hijo = actual;
                
                // Actualización de estado según tipos de comportamiento.hpp
                if (a == TURN_SR) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 1) % 8);
                else if (a == TURN_SL) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 7) % 8);
                else if (a == WALK) {
                    switch(hijo.st.brujula) {
                        case norte:    hijo.st.f--; break;
                        case noreste:  hijo.st.f--; hijo.st.c++; break;
                        case este:     hijo.st.c++; break;
                        case sureste:  hijo.st.f++; hijo.st.c++; break;
                        case sur:      hijo.st.f++; break;
                        case suroeste: hijo.st.f++; hijo.st.c--; break;
                        case oeste:    hijo.st.c--; break;
                        case noroeste: hijo.st.f--; hijo.st.c--; break;
                    }
                }

                bool viable = false;
                int coste_e = 0;
                unsigned char terr_orig = mapaResultado[actual.st.f][actual.st.c];
                int h_orig = mapaCotas[actual.st.f][actual.st.c];

                if (a == WALK) {
                    if (hijo.st.f >= 0 && hijo.st.f < (int)mapaResultado.size() && 
                        hijo.st.c >= 0 && hijo.st.c < (int)mapaResultado[0].size()) {
                        
                        unsigned char terr_dest = mapaResultado[hijo.st.f][hijo.st.c];
                        int h_dest = mapaCotas[hijo.st.f][hijo.st.c];
                        
                        // Comprobamos accesibilidad
                        if (abs(h_dest - h_orig) <= 1 && terr_dest != 'P' && terr_dest != 'M' && 
                           (terr_dest != 'B' || actual.tiene_zaps)) {
                            viable = true;
                            coste_e = GastoWALK(terr_orig, h_orig, h_dest);
                        }
                    }
                } else {
                    viable = true;
                    coste_e = GastoGiro(terr_orig);
                }

                if (viable) {
                    hijo.camino.push_back(a);
                    hijo.g = actual.g + coste_e;
                    hijo.f = hijo.g + HeuristicaT(hijo.st.f, hijo.st.c, sensores.BelPosF, sensores.BelPosC);
                    if (mapaResultado[hijo.st.f][hijo.st.c] == 'D') hijo.tiene_zaps = true;

                    pair<ubicacion, bool> estado_hijo = {hijo.st, hijo.tiene_zaps};
                    if (!cerrados.count(estado_hijo) || cerrados[estado_hijo] > hijo.g) {
                        abierta.push(hijo);
                    }
                }
            }
        }
        if (hayPlan) VisualizaPlan({sensores.posF, sensores.posC, sensores.rumbo}, plan);
    }

    if (hayPlan && !plan.empty()) {
        Action sig = plan.front();
        if (sig == WALK && sensores.agentes[2] != '_') return IDLE;
        plan.pop_front();
        return sig;
    }
    return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * Colabora con el Ingeniero para construir la red de tuberías.
 * @return Acción a realizar.
 */

// Función auxiliar para planificar ruta con mínima energía 
// Reutiliza la lógica de A* del nivel 3
list<Action> ComportamientoTecnico::PlanificarRutaN5(int target_f, int target_c, Sensores sensores) {
  list<Action> ruta;
  priority_queue<NodoBusquedaT, vector<NodoBusquedaT>, greater<NodoBusquedaT>> abierta;
  map<pair<ubicacion, bool>, int, ComparaEstado> cerrados;

  ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
  NodoBusquedaT n_inicial = {inicial, zaps, {}, 0, 0};
  n_inicial.f = HeuristicaT(inicial.f, inicial.c, target_f, target_c);
  abierta.push(n_inicial);

  while (!abierta.empty()) {
    NodoBusquedaT actual = abierta.top();
    abierta.pop();

    if (actual.st.f == target_f && actual.st.c == target_c) {
      ruta = actual.camino;
      break;
    }

    pair<ubicacion, bool> estado_actual = make_pair(actual.st, actual.tiene_zaps);
    auto it_cerrado = cerrados.find(estado_actual);
    if (it_cerrado != cerrados.end() && it_cerrado->second <= actual.g) continue;
    cerrados[estado_actual] = actual.g;

    Action acciones[] = {WALK, TURN_SR, TURN_SL};
    for (Action a : acciones) {
      NodoBusquedaT hijo = actual;
      if (a == TURN_SR) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 1) % 8);
      else if (a == TURN_SL) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 7) % 8);
      else if (a == WALK) {
        switch(hijo.st.brujula) {
          case norte: hijo.st.f--; break;
          case noreste: hijo.st.f--; hijo.st.c++; break;
          case este: hijo.st.c++; break;
          case sureste: hijo.st.f++; hijo.st.c++; break;
          case sur: hijo.st.f++; break;
          case suroeste: hijo.st.f++; hijo.st.c--; break;
          case oeste: hijo.st.c--; break;
          case noroeste: hijo.st.f--; hijo.st.c--; break;
        }
      }

      bool viable = false;
      int coste_e = 0;
      unsigned char terr_orig = mapaResultado[actual.st.f][actual.st.c];
      int h_orig = mapaCotas[actual.st.f][actual.st.c];

      if (a == WALK) {
        if (hijo.st.f >= 0 && hijo.st.f < (int)mapaResultado.size() && 
            hijo.st.c >= 0 && hijo.st.c < (int)mapaResultado[0].size()) {
          unsigned char terr_dest = mapaResultado[hijo.st.f][hijo.st.c];
          int h_dest = mapaCotas[hijo.st.f][hijo.st.c];
          if (abs(h_dest - h_orig) <= 1 && terr_dest != 'P' && terr_dest != 'M' &&
              (terr_dest != 'B' || actual.tiene_zaps)) {
            viable = true;
            coste_e = GastoWALK(terr_orig, h_orig, h_dest);
          }
        }
      } else {
        viable = true;
        coste_e = GastoGiro(terr_orig);
      }

      if (viable) {
        hijo.camino.push_back(a);
        hijo.g += coste_e;
        hijo.f = hijo.g + HeuristicaT(hijo.st.f, hijo.st.c, target_f, target_c);
        if (mapaResultado[hijo.st.f][hijo.st.c] == 'D') hijo.tiene_zaps = true;

        pair<ubicacion, bool> estado_hijo = make_pair(hijo.st, hijo.tiene_zaps);
        auto it_hijo = cerrados.find(estado_hijo);
        if (it_hijo == cerrados.end() || it_hijo->second > hijo.g) {
          cerrados[estado_hijo] = hijo.g;
          abierta.push(hijo);
        }
      }
    }
  }
  
  return ruta;
}

/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * Colabora con el Ingeniero para construir la red de tuberías.
 * @return Acción a realizar.
 */

Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D') zaps = true;
   
  // Capturar COME en cuanto llega 
  if (sensores.venpaca && tec_n5_fase == 0) {
    // Nuevo destino recibido: resetear todo y recalcular
    gof = sensores.GotoF;
    goc = sensores.GotoC;
    hayPlan = false;
    plan.clear();
    bloqueadoF = -1;
    bloqueadoC = -1;
    tec_n5_fase = 1; // Hay destino, calcular ruta
  }

  // Sin destino aún por el ingeniero, ir moviendose un poco por el mapa
    if (gof == -1){
      if(contMov < 7){
        contMov++;
        return ComportamientoTecnicoNivel_1(sensores);
      } 
      else return IDLE;
      
    }


  // Calcular A* hacia gof/goc y ejecutarlo
  if (tec_n5_fase == 1) {
    // Ya llegamos al destino
    if (sensores.posF == gof && sensores.posC == goc) {
      tec_n5_fase = 2; // Orientarse hacia el ingeniero
      hayPlan = false;
      plan.clear();
      bloqueadoF = -1;
      bloqueadoC = -1;
      return IDLE;
    }

    // Calcular plan si no lo tenemos
    if (!hayPlan) {
      plan.clear();
      priority_queue<NodoBusquedaT, vector<NodoBusquedaT>, greater<NodoBusquedaT>> abierta;
      map<pair<ubicacion, bool>, int, ComparaEstado> cerrados;

      ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
      NodoBusquedaT n_inicial = {inicial, zaps, {}, 0, 0};
      n_inicial.f = HeuristicaT(inicial.f, inicial.c, gof, goc); // <- gof/goc, no BelPos
      abierta.push(n_inicial);

      while (!abierta.empty()) {
        NodoBusquedaT actual = abierta.top();
        abierta.pop();

        if (actual.st.f == gof && actual.st.c == goc) {
          plan = actual.camino;
          hayPlan = true;
          break;
        }

        pair<ubicacion, bool> estado_actual = {actual.st, actual.tiene_zaps};
        if (cerrados.count(estado_actual) && cerrados[estado_actual] <= actual.g) continue;
        cerrados[estado_actual] = actual.g;

        Action acciones[] = {WALK, TURN_SR, TURN_SL};
        for (Action a : acciones) {
          NodoBusquedaT hijo = actual;

          if (a == TURN_SR) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 1) % 8);
          else if (a == TURN_SL) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 7) % 8);
          else if (a == WALK) {
            switch(hijo.st.brujula) {
              case norte:    hijo.st.f--; break;
              case noreste:  hijo.st.f--; hijo.st.c++; break;
              case este:     hijo.st.c++; break;
              case sureste:  hijo.st.f++; hijo.st.c++; break;
              case sur:      hijo.st.f++; break;
              case suroeste: hijo.st.f++; hijo.st.c--; break;
              case oeste:    hijo.st.c--; break;
              case noroeste: hijo.st.f--; hijo.st.c--; break;
            }
          }

          bool viable = false;
          int coste_e = 0;
          unsigned char terr_orig = mapaResultado[actual.st.f][actual.st.c];
          int h_orig = mapaCotas[actual.st.f][actual.st.c];

          if (a == WALK) {
            if (hijo.st.f >= 0 && hijo.st.f < (int)mapaResultado.size() &&
                hijo.st.c >= 0 && hijo.st.c < (int)mapaResultado[0].size()) {
              unsigned char terr_dest = mapaResultado[hijo.st.f][hijo.st.c];
              int h_dest = mapaCotas[hijo.st.f][hijo.st.c];
              if (abs(h_dest - h_orig) <= 1 && terr_dest != 'P' && terr_dest != 'M' && terr_dest != '?' &&
                 (terr_dest != 'B' || actual.tiene_zaps) &&
                 !(hijo.st.f == bloqueadoF && hijo.st.c == bloqueadoC)) {
                viable = true;
                coste_e = GastoWALK(terr_orig, h_orig, h_dest);
              }
            }
          } else {
            viable = true;
            coste_e = GastoGiro(terr_orig);
          }

          if (viable) {
            hijo.camino.push_back(a);
            hijo.g = actual.g + coste_e;
            hijo.f = hijo.g + HeuristicaT(hijo.st.f, hijo.st.c, gof, goc); // <- gof/goc
            if (mapaResultado[hijo.st.f][hijo.st.c] == 'D') hijo.tiene_zaps = true;
            pair<ubicacion, bool> estado_hijo = {hijo.st, hijo.tiene_zaps};
            if (!cerrados.count(estado_hijo) || cerrados[estado_hijo] > hijo.g)
              abierta.push(hijo);
          }
        }
      }
      if (hayPlan) VisualizaPlan({sensores.posF, sensores.posC, sensores.rumbo}, plan);
    }

    // Ejecutar plan
    if (hayPlan && !plan.empty()) {
      Action sig = plan.front();
      if (sig == WALK && sensores.agentes[2] != '_') {
        // Bloqueado por ingeniero: recalcular rodeando
        ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
        ubicacion frente = Delante(actual);
        if(frente.f != gof || frente.c != goc){
          bloqueadoF = frente.f;
          bloqueadoC = frente.c;
          hayPlan = false;
          plan.clear();
        }
        return IDLE;
      }
      plan.pop_front();
      return sig;
    }

    return IDLE;
  }

  // Ya en destino, orientarse hacia el ingeniero para INSTALL
  if (tec_n5_fase == 2) {
      if (sensores.agentes[2] == 'i') {
        g_tecnico_listo_install = true;
        if (g_ingeniero_listo_install) {
          // Ambos listos: hacer INSTALL y resetear flags
          g_ingeniero_listo_install = false;
          g_tecnico_listo_install = false;
          tec_n5_fase = 0;
          return INSTALL;
        }
        return IDLE; // Esperar al ingeniero
      }
    return TURN_SR; // Orientarse hasta ver al ingeniero
  }

  return IDLE;
}


Action ComportamientoTecnico::InvestigacionInteligenteT(Sensores sensores) {

    ActualizarMapa(sensores);

    if (!plan.empty()) {
        Action sig = plan.front();
        ubicacion pos_actual = {sensores.posF, sensores.posC, sensores.rumbo};
        bool seguro = true;
        if (sig == WALK) {
            ubicacion sig_pos = Delante(pos_actual);
            if (!CasillaTransitableT(sig_pos.f, sig_pos.c, pos_actual.f, pos_actual.c, zaps)) seguro = false;
        } 
        if (seguro) { plan.pop_front(); return sig; } 
        else { plan.clear(); }
    }

    // NODO CON PRIORIDAD
    struct Nodo {
        ubicacion loc;
        list<Action> camino;
        int prioridad; 
        bool operator>(const Nodo& otro) const { return prioridad > otro.prioridad; }
    };

    priority_queue<Nodo, vector<Nodo>, greater<Nodo>> q;
    vector<vector<vector<bool>>> visitados(mapaResultado.size(), vector<vector<bool>>(mapaResultado[0].size(), vector<bool>(8, false)));

    ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
    q.push({inicial, {}, 0});
    visitados[inicial.f][inicial.c][inicial.brujula] = true;

    while (!q.empty()) {
        Nodo actual = q.top();
        q.pop();

        // Casilla adyacente a un '?'
        bool ve_desconocido = false;
        int df[8] = {-1, -1, -1, 0, 0, 1, 1, 1}, dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
        for (int i = 0; i < 8; i++) {
            int nf = actual.loc.f + df[i], nc = actual.loc.c + dc[i];
            if (nf >= 0 && nf < mapaResultado.size() && nc >= 0 && nc < mapaResultado[0].size()) {
                if (mapaResultado[nf][nc] == '?') { ve_desconocido = true; break; }
            }
        }

        if (ve_desconocido && !actual.camino.empty()) {
            plan = actual.camino;
            Action primera = plan.front();
            plan.pop_front();
            return primera;
        }

        for (Action act : {WALK, TURN_SL, TURN_SR}) {
            ubicacion siguiente = actual.loc;
            bool posible = false;

            if (act == WALK) {
                siguiente = Delante(actual.loc);
                if (CasillaTransitableT(siguiente.f, siguiente.c, actual.loc.f, actual.loc.c, zaps) && sensores.superficie[2] != 'A') posible = true;
            } else {
                if (act == TURN_SL) siguiente.brujula = (Orientacion)(((int)siguiente.brujula + 7) % 8);
                else siguiente.brujula = (Orientacion)(((int)siguiente.brujula + 1) % 8);
                posible = true;
            }

            if (posible && !visitados[siguiente.f][siguiente.c][siguiente.brujula]) {
                visitados[siguiente.f][siguiente.c][siguiente.brujula] = true;
                
                // Si hay objetivo (gof), priorizamos la distancia al objetivo, sino, priorizamos el camino más corto.
                int h = 0;
                if (gof != -1) {
                    h = abs(gof - siguiente.f) + abs(goc - siguiente.c);
                } else {
                    h = actual.camino.size() + 1;
                }

                list<Action> nuevo_camino = actual.camino;
                nuevo_camino.push_back(act);
                q.push({siguiente, nuevo_camino, h});
            }
        }
    }
    return ComportamientoTecnicoNivel_1(sensores);
}

Action ComportamientoTecnico::NavegacionHaciaObjetivo(Sensores sensores) {
    ActualizarMapa(sensores);

    // Validar el plan existente 
    if (!plan.empty()) {
        Action sig = plan.front();
        ubicacion pos_actual = {sensores.posF, sensores.posC, sensores.rumbo};
        bool seguro = true;

        if (sig == WALK) {
            ubicacion sig_pos = Delante(pos_actual);
            if (!CasillaTransitableT(sig_pos.f, sig_pos.c, pos_actual.f, pos_actual.c, zaps) && sensores.superficie[2] != 'A') seguro = false;
        } 

        if (seguro) {
            plan.pop_front();
            return sig;
        } else {
            plan.clear();
        }
    }

    // Búsqueda de camino con prioridad  
    struct Nodo {
        ubicacion loc;
        list<Action> camino;
        int dist; // Distancia Manhattan al objetivo
        bool operator>(const Nodo& otro) const { return dist > otro.dist; }
    };

    priority_queue<Nodo, vector<Nodo>, greater<Nodo>> pq;
    
    // Matriz de visitados para no entrar en bucle
    vector<vector<vector<bool>>> visitados(
        mapaResultado.size(), 
        vector<vector<bool>>(mapaResultado[0].size(), vector<bool>(8, false))
    );

    ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
    int d_ini = abs(gof - inicial.f) + abs(goc - inicial.c);
    pq.push({inicial, {}, d_ini});

    int nodos_expandidos = 0;
    const int MAX_NODOS = 2000; // Límite para evitar lag

    while (!pq.empty() && nodos_expandidos < MAX_NODOS) {
        Nodo actual = pq.top();
        pq.pop();
        nodos_expandidos++;

        // Meta alcanzada
        if (actual.loc.f == gof && actual.loc.c == goc) {
            plan = actual.camino;
            Action primera = plan.front();
            plan.pop_front();
            return primera;
        }

        if (visitados[actual.loc.f][actual.loc.c][actual.loc.brujula]) continue;
        visitados[actual.loc.f][actual.loc.c][actual.loc.brujula] = true;

        vector<Action> acciones = {WALK, TURN_SL, TURN_SR};
        for (Action act : acciones) {
            ubicacion sig = actual.loc;
            bool posible = false;

            if (act == WALK) {
                sig = Delante(actual.loc);
                if (CasillaTransitableT(sig.f, sig.c, actual.loc.f, actual.loc.c, zaps)  && sensores.superficie[2] != 'A') posible = true;
            } else { // Giros
                if (act == TURN_SL) sig.brujula = (Orientacion)(((int)sig.brujula + 7) % 8);
                else sig.brujula = (Orientacion)(((int)sig.brujula + 1) % 8);
                posible = true;
            }

            if (posible && !visitados[sig.f][sig.c][sig.brujula]) {
                int d = abs(gof - sig.f) + abs(goc - sig.c);
                list<Action> n_camino = actual.camino;
                n_camino.push_back(act);
                pq.push({sig, n_camino, d});
            }
        }
    }

    // Para no quedarnos quietos y seguir moviéndonos hacia el objetivo.
    return InvestigacionInteligenteT(sensores);
}

/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * Colabora con el Ingeniero para construir la red de tuberías sin conocer el mapa.
 * @return Acción a realizar.
 */

Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
   
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D') zaps = true;


  // Mientras el ingeniero siga explorando/calculando (y no haya plan)
    if (g_plan_n5.empty()) {
        return InvestigacionInteligenteT(sensores); // Ayudamos a descubrir el mapa
    }
    
   
  // Capturar COME en cuanto llega  
  if (sensores.venpaca && tec_n5_fase == 0) {
    gof = sensores.GotoF;
    goc = sensores.GotoC;
    hayPlan = false;
    plan.clear();
    bloqueadoF = -1;
    bloqueadoC = -1;
    tec_n5_fase = 1; // Hay destino, calcular ruta
  }

  // Sin destino aún por el ingeniero, ir moviendose un poco por el mapa
    if (gof == -1){
      if(contMov < 7){
        contMov++;
        return InvestigacionInteligenteT(sensores);
      } 
      else return IDLE;
      
    }


  //  Calcular A* hacia gof/goc y ejecutarlo
  if (tec_n5_fase == 1) {
    // Ya llegamos al destino
    if (sensores.posF == gof && sensores.posC == goc) {
      tec_n5_fase = 2; // Orientarse hacia el ingeniero
      hayPlan = false;
      plan.clear();
      bloqueadoF = -1;
      bloqueadoC = -1;
      return IDLE;
    }

    // Calcular plan si no lo tenemos
    if (!hayPlan && (mapaResultado[gof][goc] != '?')) {
      plan.clear();
      priority_queue<NodoBusquedaT, vector<NodoBusquedaT>, greater<NodoBusquedaT>> abierta;
      map<pair<ubicacion, bool>, int, ComparaEstado> cerrados;

      ubicacion inicial = {sensores.posF, sensores.posC, sensores.rumbo};
      NodoBusquedaT n_inicial = {inicial, zaps, {}, 0, 0};
      n_inicial.f = HeuristicaT(inicial.f, inicial.c, gof, goc); // <- gof/goc, no BelPos
      abierta.push(n_inicial);

      while (!abierta.empty()) {
        NodoBusquedaT actual = abierta.top();
        abierta.pop();

        if (actual.st.f == gof && actual.st.c == goc) {
          plan = actual.camino;
          hayPlan = true;
          break;
        }

        pair<ubicacion, bool> estado_actual = {actual.st, actual.tiene_zaps};
        if (cerrados.count(estado_actual) && cerrados[estado_actual] <= actual.g) continue;
        cerrados[estado_actual] = actual.g;

        Action acciones[] = {WALK, TURN_SR, TURN_SL};
        for (Action a : acciones) {
          NodoBusquedaT hijo = actual;

          if (a == TURN_SR) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 1) % 8);
          else if (a == TURN_SL) hijo.st.brujula = static_cast<Orientacion>((hijo.st.brujula + 7) % 8);
          else if (a == WALK) {
            switch(hijo.st.brujula) {
              case norte:    hijo.st.f--; break;
              case noreste:  hijo.st.f--; hijo.st.c++; break;
              case este:     hijo.st.c++; break;
              case sureste:  hijo.st.f++; hijo.st.c++; break;
              case sur:      hijo.st.f++; break;
              case suroeste: hijo.st.f++; hijo.st.c--; break;
              case oeste:    hijo.st.c--; break;
              case noroeste: hijo.st.f--; hijo.st.c--; break;
            }
          }

          bool viable = false;
          int coste_e = 0;
          unsigned char terr_orig = mapaResultado[actual.st.f][actual.st.c];
          int h_orig = mapaCotas[actual.st.f][actual.st.c];

          if (a == WALK) {
            if (hijo.st.f >= 0 && hijo.st.f < (int)mapaResultado.size() &&
                hijo.st.c >= 0 && hijo.st.c < (int)mapaResultado[0].size()) {
              unsigned char terr_dest = mapaResultado[hijo.st.f][hijo.st.c];
              int h_dest = mapaCotas[hijo.st.f][hijo.st.c];
              if (abs(h_dest - h_orig) <= 1 && terr_dest != 'P' && terr_dest != 'M' && terr_dest != '?' &&
                 (terr_dest != 'B' || actual.tiene_zaps) &&
                 !(hijo.st.f == bloqueadoF && hijo.st.c == bloqueadoC)) {
                viable = true;
                coste_e = GastoWALK(terr_orig, h_orig, h_dest);
              }
            }
          } else {
            viable = true;
            coste_e = GastoGiro(terr_orig);
          }

          if (viable) {
            hijo.camino.push_back(a);
            hijo.g = actual.g + coste_e;
            hijo.f = hijo.g + HeuristicaT(hijo.st.f, hijo.st.c, gof, goc); // <- gof/goc
            if (mapaResultado[hijo.st.f][hijo.st.c] == 'D') hijo.tiene_zaps = true;
            pair<ubicacion, bool> estado_hijo = {hijo.st, hijo.tiene_zaps};
            if (!cerrados.count(estado_hijo) || cerrados[estado_hijo] > hijo.g)
              abierta.push(hijo);
          }
        }
      }
      if (hayPlan) VisualizaPlan({sensores.posF, sensores.posC, sensores.rumbo}, plan);
    }

    // Ejecutar plan
    if (hayPlan && !plan.empty()) {
      Action sig = plan.front();
      if (sig == WALK && sensores.agentes[2] != '_') {
        // Bloqueado por ingeniero: recalcular rodeando
        ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
        ubicacion frente = Delante(actual);
        if(frente.f != gof || frente.c != goc){
          bloqueadoF = frente.f;
          bloqueadoC = frente.c;
          hayPlan = false;
          plan.clear();
        }
        return IDLE;
      }
      plan.pop_front();
      return sig;
    }
    else{ return NavegacionHaciaObjetivo(sensores);}

    return IDLE;
  }

  // Ya en destino, orientarse hacia el ingeniero para INSTALL
  if (tec_n5_fase == 2) {
      if (sensores.agentes[2] == 'i') {
        g_tecnico_listo_install = true;
        if (g_ingeniero_listo_install) {
          // Ambos listos: hacer INSTALL y resetear flags
          g_ingeniero_listo_install = false;
          g_tecnico_listo_install = false;
          tec_n5_fase = 0;
          return INSTALL;
        }
        return IDLE; // Esperar al ingeniero
      }
    return TURN_SR; // Orientarse hasta ver al ingeniero
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
void ComportamientoTecnico::ActualizarMapa(Sensores sensores) {
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo) {
    case norte:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
 * @brief Determina si una casilla es transitable para el tÃ©cnico.
 * En esta prÃ¡ctica, si el tÃ©cnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) return false;
  return es_camino(mapaResultado[f][c]);  // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el tÃ©cnico: desnivel mÃ¡ximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

/**
 * @brief Devuelve la posiciÃ³n (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal segÃºn la orientaciÃ³n actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--; break;                        // norte
    case 1: delante.f--; delante.c++; break;     // noreste
    case 2: delante.c++; break;                     // este
    case 3: delante.f++; delante.c++; break;     // sureste
    case 4: delante.f++; break;                        // sur
    case 5: delante.f++; delante.c--; break;     // suroeste
    case 6: delante.c--; break;                     // oeste
    case 7: delante.f--; delante.c--; break;     // noroeste
  }
  return delante;
}


/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
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
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualizaciÃ³n en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
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


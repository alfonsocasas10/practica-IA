#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <chrono>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <time.h>

#include "comportamientos/comportamiento.hpp"

extern bool g_ingeniero_listo_install;
extern bool g_tecnico_listo_install;
extern vector<Paso> g_plan_n5;

class ComportamientoIngeniero : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
    ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size) {
      zaps = false;
      last_action = IDLE;
      contador_giros =0;
      giro_preferido = TURN_SR;
      last_f = -1;
      last_c = -1;
      en_bloqueo = false;
      en_bloqueo_J = false;
      en_bloqueo_U = false;
      walk_left = true;
      giro_defecto = false;
      cont_walk = 0;

      hayPlan = false;

      tramo_actual_N5 = 0;
      recien_instalado_N5 = false;
      tramo_n5 =0;
      listo_n5 = false;
      fase_n5 = 0;
      current_tramo_n5 = 0;
      giros_180_n5 = 0;

      enPos = false;
      contRuta = 1;
      sig_f = -1;
      sig_c = -1;
    }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR, 
                         std::vector<std::vector<unsigned char>> mapaC): 
                         Comportamiento(mapaR, mapaC) {
    zaps = false;
    last_action = IDLE;
    contador_giros =0;
    giro_preferido = TURN_SL;
    last_f = -1;
    last_c = -1;
    en_bloqueo = false;
    en_bloqueo_J = false;
    en_bloqueo_U = false;
    walk_left = true;
    giro_defecto = false;
    cont_walk = 0;

    hayPlan = false;

    tramo_actual_N5 = 0;
    recien_instalado_N5 = false;
    tramo_n5 =0;
    listo_n5 = false;
    fase_n5 = 0;
    current_tramo_n5 = 0;

    enPos = false;
    contRuta = 1;
    sig_f = -1;
    sig_c = -1;
  }

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  /**
   * @brief Bucle principal de decisión del agente.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoIngeniero *clone() {
    return new ComportamientoIngeniero(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  // Funciones específicas para cada nivel (para ser implementadas por el alumno)
  
  /**
   * @brief Implementación del Nivel 0.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_0(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 1.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_1(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 2.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */ 
  Action ComportamientoIngenieroNivel_2(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 3.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_3(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 4.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_4(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 5.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_5(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 6.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza la información del mapa interno basándose en los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores (casilla actual + 15 casillas alrededor).
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Comprueba si una casilla es transitable.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee zapatillas.
   * @return true si la casilla es transitable (no es muro ni precipicio).
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLAS: Desnivel máximo 1 sin zapatillas, 2 con zapatillas.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);

  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  /**
   * @brief Comprueba si una celda es de tipo transitable por defecto (nivel 0).
   * @param c Carácter que representa el tipo de superficie.
   * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
   */
  bool es_camino(unsigned char c) const;

  // MIS FUNCIONES

  /**
   * @brief Devuelve verdadero si la casilla es transitable (nivel 1, a excepción de hierba y de agua).
   * @param c La casilla.
   * @return Verdadero si la casilla es transitable (nivel 1).
   */
  bool es_camino1(unsigned char c) const;

  /**
   * @brief Devuelve la direccion en la que se encuentra la casilla con mas prioridad (nivel 0).
   * @param i La casilla de arriba-izq.
   * @param c La casilla de delante.
   * @param i La casilla de arriba-der.
   * @param zaps Si el agente tiene zapatillas o no.
   * @return 1 si es arriba-izq, 2 si es centro, 3 si es arriba-der, 0 si no son transitables.
   */
  int veoCasillaInteresanteI0(char i, char c, char d, bool zaps);

  /**
   * @brief Devuelve la direccion en la que se encuentra la casilla con mas prioridad (nivel 1).
   * @param i La casilla de arriba-izq.
   * @param c La casilla de delante.
   * @param i La casilla de arriba-der.
   * @param zaps Si el agente tiene zapatillas o no.
   * @return 1 si es arriba-izq, 2 si es centro, 3 si es arriba-der, 0 si no son transitables.
   */
  int veoCasillaInteresanteI1(char i, char c, char d, bool zaps);

  /**
   * @brief Devuelve la casilla si está a una altura adecuada, y devuelve 'P' si no.
   * @param casilla La casilla a evaluar.
   * @param dif La diferencia de altura entre mi casilla actual y la evaluada.
   * @param zap Si el agente tiene zapatillas o no.
   * @return La casilla si está a una altura adecuada, y devuelve 'P' si no.
   */
  char viablePorAlturaI(char casilla, int dif, bool zap);

  /**
   * @brief Devuelve la casilla si es transitable.
   * @return La casilla si está a una altura adecuada, y devuelve 'P' si no.
   */
  bool CasillaTransitableI(int f, int c, int f_ant, int c_ant, bool tiene_zaps);
  
  /**
   * @brief Simula si una accion y te devuelve la siguiente ubicacion.
   * @param actual Ubicacion actual.
   * @param a Accion a realizar.
   * @return La siguiente ubicacion.
   */
  ubicacion SimularAccionI(ubicacion actual, Action a);

  /**
   * @brief Comportamiento diseñado especialmente para el nivel 6, prioriza investigar las casillas '?'.
   * @param sensores Sensores del agente.
   * @return La siguiente accion a realizar.
   */
  Action InvestigacionInteligenteI(Sensores sensores);


  // FIN MIS FUNCIONES

  /**
 * @brief Imprime por consola la secuencia de acciones de un plan para un agente.
 * @param plan  Lista de acciones del plan.
 */
  void PintaPlan(const list<Action> &plan);


/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 * @param plan  Lista de pasos (fila, columna, operación).
 */
  void PintaPlan(const list<Paso> &plan);


  /**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa gráfico.
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

  /**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
  void VisualizaRedTuberias(const list<Paso> &plan);

  bool puedeSaltarI(const Sensores &sensores, bool tiene_zaps);



private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================

  bool zaps;
  Action last_action;
  int contador_giros;
  int cont_walk;
  bool walk_left;
  Action giro_preferido;
  int last_f;
  int last_c;
  bool en_bloqueo;
  bool en_bloqueo_J;
  bool en_bloqueo_U;
  bool giro_defecto;
  vector<vector<int>> visitas;
  bool hayPlan;
  list<Action> plan;
  vector<pair<int, int>> plan_tuberias_N5;
  int tramo_actual_N5;
  list<pair<int,int>> plan_navegacion_N5;
  bool recien_instalado_N5;
  vector<pair<int,int>> plan_n5;
  int tramo_n5;
  bool listo_n5;
  int fase_n5;
  int current_tramo_n5;
  int giros_180_n5; // Contador de giros para 180°
  set<pair<int,int>> forbidden_cells;
  bool enPos;
  int contRuta;
  int sig_f;
  int sig_c;
  int sig_op;
  list<int> ops;



  // Estructura para el planificador (Búsqueda en anchura)
  struct NodoBusquedaI {
    ubicacion st;
  bool tiene_zaps;
  list<Action> camino;

  // El operator< es fundamental para que el SET de 'cerrada' no ignore 
  // estados donde el agente ahora tiene zapatillas pero antes no.
  bool operator<(const NodoBusquedaI &otro) const {
    if (st.f != otro.st.f) return st.f < otro.st.f;
    if (st.c != otro.st.c) return st.c < otro.st.c;
    if (st.brujula != otro.st.brujula) return st.brujula < otro.st.brujula;
    return tiene_zaps < otro.tiene_zaps;
  }
  };

// Estructuras necesarias para el BFS
struct EstadoTuberia {
    int f, c, h_tuberia;
    // Indispensable para usar std::map de C++ de forma segura
    bool operator<(const EstadoTuberia& o) const {
        if (f != o.f) return f < o.f;
        if (c != o.c) return c < o.c;
        return h_tuberia < o.h_tuberia;
    }
};



struct NodoTuberia {
    EstadoTuberia st;
    list<Paso> camino;
    int eco;
    int longitud;  // NUEVO
    int op;
    bool operator>(const NodoTuberia& o) const { return longitud > o.longitud; }  // ordenar por longitud
};

};

#endif

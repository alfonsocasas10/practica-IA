#ifndef COMPORTAMIENTOTECNICO_H
#define COMPORTAMIENTOTECNICO_H

#include <chrono>
#include <time.h>
#include <thread>
#include <list>

#include "comportamientos/comportamiento.hpp"

extern bool g_ingeniero_listo_install;
extern bool g_tecnico_listo_install;
extern vector<Paso> g_plan_n5;


// =========================================================================
// DOCUMENTACIÓN PARA ESTUDIANTES
// =========================================================================
/*
 * CLASE: ComportamientoTecnico
 * 
 * DESCRIPCIÓN:
 * Esta clase implementa el comportamiento del agente Técnico en el mundo Belkan.
 * El técnico colabora con el ingeniero para resolver el problema de instalación de tuberías
 */



class ComportamientoTecnico : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoTecnico(unsigned int size = 0) : Comportamiento(size) {
    zaps = false;
    last_action = IDLE;
    contador_giros =0;
    giro_preferido = TURN_SL;
    last_f = -1;
    last_c = -1;
    en_bloqueo = false;
    en_bloqueo_U = false;
    walk_left = true;
    giro_defecto = false;
    cont_walk = 0;

    //Nivel E
    hayPlan = false;

    tramo_actual_N5 = 0;
    recien_instalado_N5 = false;
    tramo_n5 = 0;
    listo_n5 = false;
    tec_n5_esperando = true;
    tec_n5_ruta.clear();
    tec_n5_fase = 0;
    tec_n5_target_f = -1;
    tec_n5_target_c = -1;
    gof = -1;
    goc = -1;
    giros_180_tec_n5 = 0;
    contMov = 0;
   }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoTecnico(std::vector<std::vector<unsigned char>> mapaR, 
                       std::vector<std::vector<unsigned char>> mapaC): 
                       Comportamiento(mapaR, mapaC) {
    zaps = false;
    last_action = IDLE;
    contador_giros =0;
    giro_preferido = TURN_SL;
    last_f = -1;
    last_c = -1;
    en_bloqueo = false;
    en_bloqueo_U = false;
    walk_left = true;
    giro_defecto = false;
    cont_walk = 0;

    //Nivel E
    hayPlan = false;

    tramo_actual_N5 = 0;
    recien_instalado_N5 = false;
    tramo_n5 =0;
    listo_n5 = false;
    tec_n5_esperando = true;
    tec_n5_ruta.clear();
    tec_n5_fase = 0;
    tec_n5_target_f = -1;
    tec_n5_target_c = -1;
    giros_180_tec_n5 = 0;
  
      gof = -1;
      goc = -1;
      bloqueadoF = -1;
      bloqueadoC = -1;
      contMov = 0;
   }

  ComportamientoTecnico(const ComportamientoTecnico &comport): Comportamiento(comport) {}
  ~ComportamientoTecnico() {}

  /**
   * @brief Bucle principal de decisión del técnico.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoTecnico *clone() {
    return new ComportamientoTecnico(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================
  
/**
 * @brief Comportamiento del técnico para el Nivel 0.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_0(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_1(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_2(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_3(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_4(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_5(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_6(Sensores sensores);

  /**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_E(Sensores sensores);

  // Función auxiliar para planificar ruta con mínima energía (nivel 3)
  list<Action> PlanificarRutaN5(int target_f, int target_c, Sensores sensores);
 
protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores.
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Determina si una casilla es transitable para el técnico.
   * NOTA: El técnico puede tener reglas de transitabilidad diferentes al ingeniero.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee las zapatillas.
   * @return true si la casilla es transitable.
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLA PARA TÉCNICO: Desnivel máximo siempre 1 (independiente de zapatillas).
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual);

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
   * @brief Devuelve verdadero si la casilla es transitable (nivel 1, a excepción agua).
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
  int veoCasillaInteresanteT0(char i, char c, char d, bool zaps);

  /**
   * @brief Devuelve la direccion en la que se encuentra la casilla con mas prioridad (nivel 1).
   * @param i La casilla de arriba-izq.
   * @param c La casilla de delante.
   * @param i La casilla de arriba-der.
   * @param zaps Si el agente tiene zapatillas o no.
   * @return 1 si es arriba-izq, 2 si es centro, 3 si es arriba-der, 0 si no son transitables.
   */
  int veoCasillaInteresanteT1(char i, char c, char d, bool zaps);
  
  /**
   * @brief Devuelve la casilla si está a una altura adecuada, y devuelve 'P' si no.
   * @param casilla La casilla a evaluar.
   * @param dif La diferencia de altura entre mi casilla actual y la evaluada.
   * @return La casilla si está a una altura adecuada, y devuelve 'P' si no.
   */
  char viablePorAlturaT(char casilla, int dif);

  /**
   * @brief Observa que casilla es preferible.
   * @return El numero indicando cual es mejor.
   */
  int veoCasillaExplorarT(bool vi, bool vc, bool vd, char i, char c, char d);

  
  /**
   * @brief Simula si una accion y te devuelve la siguiente ubicacion.
   * @param actual Ubicacion actual.
   * @param a Accion a realizar.
   * @return La siguiente ubicacion.
   */
  ubicacion SimularAccionT(ubicacion actual, Action a);

  /**
   * @brief Devuelve la casilla si es transitable.
   * @return La casilla si está a una altura adecuada, y devuelve 'P' si no.
   */
  bool CasillaTransitableT(int f, int c, int f_ant, int c_ant, bool con_zaps);
  
  /**
   * @brief Comportamiento diseñado especialmente para el nivel 6, prioriza investigar las casillas '?'.
   * @param sensores Sensores del agente.
   * @return La siguiente accion a realizar.
   */
  Action InvestigacionInteligenteT(Sensores sensores);

  /**
   * @brief Comportamiento diseñado especialmente para el nivel 6, investiga avanzando hacia gof y goc.
   * @param sensores Sensores del agente.
   * @return La siguiente accion a realizar.
   */
  Action NavegacionHaciaObjetivo(Sensores sensores);

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
  bool tec_n5_esperando;
  list<Action> tec_n5_ruta;
  int tec_n5_fase; // 0: esperando ingeniero, 1: esperando COME, 2: planificando, 3: ejecutando ruta, 4: orientando e INSTALL
  int tec_n5_target_f, tec_n5_target_c;
  int gof;
  int goc;
  int contMov;
  int bloqueadoF;
  int bloqueadoC;
  int giros_180_tec_n5;

 
  struct NodoBusquedaT {
    ubicacion st;
    bool tiene_zaps;
    list<Action> camino;
    int g; // Energía gastada
    int f; // g + h

    // La cola de prioridad necesita el menor f arriba
    bool operator>(const NodoBusquedaT &otro) const {
        return f > otro.f;
    }
  };

  // Comparador para que el std::map acepte ubicacion + tiene_zaps
  struct ComparaEstado {
      bool operator()(const pair<ubicacion, bool> &a, const pair<ubicacion, bool> &b) const {
          if (a.first.f != b.first.f) return a.first.f < b.first.f;
          if (a.first.c != b.first.c) return a.first.c < b.first.c;
          if (a.first.brujula != b.first.brujula) return a.first.brujula < b.first.brujula;
          return a.second < b.second;
      }
  };

};

#endif

#!/bin/bash

echo "======================================================"
echo " INICIANDO BATERÍA DE PRUEBAS - NIVEL 6"
echo "======================================================"

echo -e "\n---> [1/16] minivertiguito.map (O 19 11)"
./build/practica2SG -m ./mapas/minivertiguito.map -n 6 -i 16 8 3 -t 26 9 0 -seed 0 -Tiempo 3000 -Ambiental 648 -Energia 3000 -O 19 11

echo -e "\n---> [2/16] minivertiguito.map (O 17 22)"
./build/practica2SG -m ./mapas/minivertiguito.map -n 6 -i 16 8 3 -t 26 9 0 -seed 0 -Tiempo 3000 -Ambiental 1000 -Energia 3000 -O 17 22

echo -e "\n---> [3/16] minivertiguito.map (O 3 3)"
./build/practica2SG -m ./mapas/minivertiguito.map -n 6 -i 16 8 3 -t 26 9 0 -seed 0 -Tiempo 3000 -Ambiental 1000 -Energia 3000 -O 3 3

echo -e "\n---> [4/16] mapa30_26.map"
./build/practica2SG -m ./mapas/mapa30_26.map -n 6 -i 13 26 5 -t 16 9 6 -seed 0 -Tiempo 3000 -Ambiental 2364 -Energia 3000 -O 26 3

echo -e "\n---> [5/16] vertigo.map"
./build/practica2SG -m ./mapas/vertigo.map -n 6 -i 80 75 2 -t 56 24 5 -seed 0 -Tiempo 3000 -Ambiental 2688 -Energia 9517 -O 4 36

echo -e "\n---> [6/16] mapa30.map"
./build/practica2SG -m ./mapas/mapa30.map -n 6 -i 25 20 6 -t 26 23 6 -seed 0 -Tiempo 3000 -Ambiental 1804 -Energia 6361 -O 14 17

echo -e "\n---> [7/16] mapa50_cuadricula.map (Ambiental 1533)"
./build/practica2SG -m ./mapas/mapa50_cuadricula.map -n 6 -i 25 38 6 -t 15 17 1 -seed 0 -Tiempo 3000 -Ambiental 1533 -Energia 4092 -O 30 24

echo -e "\n---> [8/16] mapa75_espirales.map"
./build/practica2SG -m ./mapas/mapa75_espirales.map -n 6 -i 69 69 7 -t 3 69 5 -seed 0 -Tiempo 3000 -Ambiental 1417 -Energia 4150 -O 16 10

echo -e "\n---> [9/16] mapa50_cuadricula.map (Ambiental 3533)"
./build/practica2SG -m ./mapas/mapa50_cuadricula.map -n 6 -i 25 38 6 -t 15 17 1 -seed 0 -Tiempo 3000 -Ambiental 3533 -Energia 4092 -O 46 18

echo -e "\n---> [10/16] mapa75.map (Ambiental 865)"
./build/practica2SG -m ./mapas/mapa75.map -n 6 -i 29 68 6 -t 14 52 2 -seed 0 -Tiempo 3000 -Ambiental 865 -Energia 5574 -O 52 32

echo -e "\n---> [11/16] mapa100.map"
./build/practica2SG -m ./mapas/mapa100.map -n 6 -i 13 33 3 -t 82 73 6 -seed 0 -Tiempo 3000 -Ambiental 1719 -Energia 4581 -O 25 61

echo -e "\n---> [12/16] mapa75.map (Ambiental 1500)"
./build/practica2SG -m ./mapas/mapa75.map -n 6 -i 29 68 6 -t 14 52 2 -seed 0 -Tiempo 3000 -Ambiental 1500 -Energia 5574 -O 48 16

echo -e "\n---> [13/16] mapaop.map"
./build/practica2SG -m ./mapas/mapaop.map -n 6 -i 42 36 1 -t 42 37 1 -seed 0 -Tiempo 3000 -Ambiental 2280 -Energia 3403 -O 26 22

echo -e "\n---> [14/16] islas_cambio_climatico.map"
./build/practica2SG -m ./mapas/islas_cambio_climatico.map -n 6 -i 27 94 7 -t 52 92 4 -seed 0 -Tiempo 3000 -Ambiental 2107 -Energia 4383 -O 11 10

echo -e "\n---> [15/16] mapa50.map"
./build/practica2SG -m ./mapas/mapa50.map -n 6 -i 17 27 6 -t 41 23 1 -seed 0 -Tiempo 3000 -Ambiental 2836 -Energia 3699 -O 32 16

echo -e "\n---> [16/16] gemini2.map"
./build/practica2SG -m ./mapas/gemini2.map -n 6 -i 10 21 1 -t 20 4 2 -seed 0 -Tiempo 3000 -Ambiental 1500 -Energia 3000 -O 26 3

echo "======================================================"
echo " BATERÍA DE PRUEBAS COMPLETADA"
echo "======================================================"

#!/bin/bash

echo " ================================================================ "
echo " ================== ENTREGABLE 2 ================================"
echo " ================================================================ "

echo " ======== 1 con openmp  "
gcc -fopenmp -o uno_openmp.o E2-1-openmp.c
for size in 512 1024 2048
do
  echo " ==== Matriz $((size)) "
  for thread in 4
  do
    echo " ----> Threads -> $thread"
    ./uno_openmp.o $size $thread
  done
done

echo " ======== 1 con pthread "
gcc -pthread -o uno_pthread.o E2-1-pthread.c
for size in 512 1024 2048
do
  echo " ==== Matriz $((size)) "
  for thread in 4
  do
    echo " ----> Threads -> $thread"
    ./uno_pthread.o $size $thread
  done
done

echo " ======== 1 secuencial "
gcc -o uno_secuencial.o E2-1-sec.c
for matriz in 512 1024 2048
do
  echo " ==== Matriz $((matriz)) "
  for cantBloques in 32 64
  do
    tamBloque=$((matriz/cantBloques))
    echo " ----> Cantidad bloques -> $cantBloques Tamano bloque -> $tamBloque"
    ./uno_secuencial.o $cantBloques $tamBloque 0
    tamBloque=$((tamBloque/2))
  done
done

echo " ======== 2 mergesort - con 4 hilos"
gcc -o E2-2.o E2-2.c
./E2-2.o 100000000 100 4 0

echo " ================================================================ "
echo " ==================== ENTREGABLE 3 =============================="
echo " ================================================================ "
echo "  Ejercicio 1 "
mpicc -o E3-1.o E3-1.c

mpirun -np 4 E3-1.o 512
mpirun -np 4 E3-1.o 1024
mpirun -np 4 E3-1.o 2048

echo " ================== "
echo " Ejercicio 2 "
mpicc -o E3-2.o E3-2.c

mpirun -np 4 E3-2.o 512 100 0
mpirun -np 4 E3-2.o 1024 100 0
mpirun -np 4 E3-2.o 2048 100 0

echo " ================== "
echo " Ejercicio 3 "
gcc -o E3-3-sec.o E3-3-sec.c

echo " Secuencial "
./E3-3.sec.o 1000000000 100 0

echo " Paralelo - MPI - En progreso "
# mpicc -o ejercicio_3.o ejercicio_3.c

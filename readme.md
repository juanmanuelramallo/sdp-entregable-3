# Sistemas Paralelos y Distribuidos
## Entregable 3
#### Juan Manuel Ramallo 954/1

## 1.
_Realizar un algoritmo MPI que resuelva la expresión: 𝑀 = 𝑑̅𝐴𝐵𝐶 + 𝑏̅𝐷𝐸𝐹, donde A, B, C, D, E y F son matrices de NxN. d̅ y b̅ son los promedios de los valores de los elementos de las matrices D y B, respectivamente.
Evaluar N=512, 1024 y 2048._

Primero que nada para mayor legibilidad y poder entender mejor el flujo del programa, se separan las acciones que deben ejecutar el proceso root, o maestro, de los procesos workers.
El root se encarga de inicializar y alocar memoria para las matrices a usar.

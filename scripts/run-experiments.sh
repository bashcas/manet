#!/bin/bash

# Obtener el directorio raíz del proyecto
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
DATA_DIR="$ROOT_DIR/data"

# Asegurar que el directorio de datos existe
mkdir -p "$DATA_DIR"

# Eliminar resultados anteriores
rm -f "$DATA_DIR/manet-results.csv"

# Parámetros base
MEMBERS=(3 5 7)           # Nodos por cluster
TIMES=(100 200 300)       # Tiempos de simulación
RUNS=5                    # Repeticiones por configuración

# Asegurarse de estar en el directorio de NS-3
cd ../ns-3-dev || exit 1

for members in "${MEMBERS[@]}"; do
  for simtime in "${TIMES[@]}"; do
    for run in $(seq 1 $RUNS); do
      echo "Running simulation with members=$members time=$simtime run=$run"
      ./ns3 run "scratch/manet --members=$members --time=$simtime --run=$run --csv=$DATA_DIR/manet-results.csv --append=1"
    done
  done
done

echo "All simulations completed. Results in $DATA_DIR/manet-results.csv"

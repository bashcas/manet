#!/bin/bash

# Obtener el directorio raíz del proyecto
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
DATA_DIR="$ROOT_DIR/data"
NS3_DIR="$ROOT_DIR/ns-3-dev"

# Verificar que NS-3 está instalado
if [ ! -d "$NS3_DIR" ]; then
    echo "Error: NS-3 no encontrado en $NS3_DIR"
    echo "Por favor, clone NS-3 dentro del directorio del proyecto:"
    echo "git clone https://gitlab.com/nsnam/ns-3-dev.git"
    exit 1
fi

# Asegurar que el directorio de datos existe
mkdir -p "$DATA_DIR"

# Eliminar resultados anteriores
rm -f "$DATA_DIR/manet-results.csv"

# Copiar manet.cc a ns-3-dev/scratch si no existe o ha sido modificado
if [ ! -f "$NS3_DIR/scratch/manet.cc" ] || [ "$ROOT_DIR/manet.cc" -nt "$NS3_DIR/scratch/manet.cc" ]; then
    cp "$ROOT_DIR/manet.cc" "$NS3_DIR/scratch/"
    echo "Copiado manet.cc a scratch/"
fi

# Parámetros base
MEMBERS=(3 5 7)           # Nodos por cluster
TIMES=(100 200 300)       # Tiempos de simulación
RUNS=5                    # Repeticiones por configuración

# Asegurarse de estar en el directorio de NS-3
cd "$NS3_DIR" || exit 1

for members in "${MEMBERS[@]}"; do
  for simtime in "${TIMES[@]}"; do
    for run in $(seq 1 $RUNS); do
      echo "Running simulation with members=$members time=$simtime run=$run"
      ./ns3 run "scratch/manet --members=$members --time=$simtime --run=$run --csv=$DATA_DIR/manet-results.csv --append=1"
    done
  done
done

echo "All simulations completed. Results in $DATA_DIR/manet-results.csv"

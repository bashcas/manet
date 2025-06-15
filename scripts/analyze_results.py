#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path

# Configurar rutas
ROOT_DIR = Path(__file__).parent.parent
DATA_DIR = ROOT_DIR / "data"
PLOTS_DIR = ROOT_DIR / "plots"

# Asegurar que los directorios existan
DATA_DIR.mkdir(exist_ok=True)
PLOTS_DIR.mkdir(exist_ok=True)

# Configurar estilo de las gráficas
plt.style.use('default')  # Usar estilo por defecto en lugar de seaborn
sns.set_theme(style="whitegrid")  # Configuración más compatible de seaborn

# Leer datos (ahora usando los nombres de columnas del CSV)
df = pd.read_csv(DATA_DIR / 'manet-results.csv')

# Extraer información de las direcciones IP para clasificar los flujos
df['is_inter_cluster'] = df.apply(
    lambda row: row['src_addr'].split('.')[2] != row['dst_addr'].split('.')[2],
    axis=1
)

# Métricas a analizar
metrics = ['throughput_mbps', 'delay_seconds', 'hop_count', 'pdr']

# 1. PDR vs Número de nodos (separado por tráfico inter/intra cluster)
plt.figure(figsize=(10, 6))
sns.boxplot(data=df, x='run', y='pdr', hue='is_inter_cluster')
plt.title('Packet Delivery Ratio por Tipo de Tráfico')
plt.xlabel('Run Number')
plt.ylabel('PDR')
plt.savefig(PLOTS_DIR / 'pdr_analysis.png')
plt.close()

# 2. CDF del retardo
plt.figure(figsize=(10, 6))
for is_inter in [True, False]:
    data = df[df['is_inter_cluster'] == is_inter]['delay_seconds']
    data = data[data > 0]  # Filtrar valores válidos
    if not data.empty:
        sns.ecdfplot(data=data, label='Inter-cluster' if is_inter else 'Intra-cluster')
plt.title('CDF del Retardo')
plt.xlabel('Retardo (segundos)')
plt.ylabel('CDF')
plt.legend()
plt.savefig(PLOTS_DIR / 'delay_cdf.png')
plt.close()

# 3. Distribución de saltos
plt.figure(figsize=(10, 6))
sns.histplot(data=df[df['hop_count'] > 0], x='hop_count', hue='is_inter_cluster',
            multiple="layer", stat='density', discrete=True)
plt.title('Distribución del Número de Saltos')
plt.xlabel('Número de Saltos')
plt.ylabel('Densidad')
plt.savefig(PLOTS_DIR / 'hop_distribution.png')
plt.close()

# 4. Throughput promedio por tipo de tráfico
plt.figure(figsize=(10, 6))
sns.boxplot(data=df[df['throughput_mbps'] > 0], x='run', y='throughput_mbps',
            hue='is_inter_cluster')
plt.title('Throughput por Tipo de Tráfico')
plt.xlabel('Run Number')
plt.ylabel('Throughput (Mbps)')
plt.yscale('log')  # Escala logarítmica para mejor visualización
plt.savefig(PLOTS_DIR / 'throughput_analysis.png')
plt.close()

# Generar resumen estadístico
print("\nResumen Estadístico por Tipo de Tráfico:")
summary = df.groupby('is_inter_cluster')[metrics].agg(['mean', 'std', 'min', 'max'])
print(summary.round(4))

# Guardar resumen en CSV
summary.to_csv(DATA_DIR / 'summary_stats.csv')

# MANET Performance Analysis

This repository contains the analysis of a hierarchical MANET (Mobile Ad-hoc Network) simulation using NS-3, focusing on the impact of hierarchical mobility on AODV protocol performance.

## Prerequisites

### NS-3 Setup

1. Install NS-3 (version 3.27 or later):

   ```bash
   # Clone NS-3
   git clone https://gitlab.com/nsnam/ns-3-dev.git
   cd ns-3-dev

   # Configure and build NS-3
   ./ns3 configure --enable-examples --enable-tests
   ./ns3 build
   ```

2. Copy simulation file:
   - Copy `manet.cc` to the `ns-3-dev/scratch/` directory
   - This file contains the MANET simulation configuration

## Directory Structure

```
manet_results/
├── analysis/        # Detailed analysis and conclusions
├── data/           # Raw simulation data and statistics
├── plots/          # Generated visualizations
└── scripts/        # Analysis and simulation scripts
```

## Python Analysis Requirements

- Python 3.x
- Required Python packages are listed in `requirements.txt`

## Installation

1. Create a Python virtual environment:

   ```bash
   python -m venv venv
   source venv/bin/activate  # On Unix/macOS
   # or
   .\venv\Scripts\activate  # On Windows
   ```

2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

## Running the Simulation

1. From the ns-3-dev directory:

   ```bash
   ./ns3 run "scratch/manet --members=5 --time=200 --run=1"
   ```

   Parameters:

   - `members`: Number of nodes per cluster (default: 5)
   - `time`: Simulation time in seconds (default: 200)
   - `run`: Run number for random seed (default: 1)
   - `tracing`: Enable PCAP tracing (default: false)
   - `append`: Append to existing CSV (default: false)

2. For multiple runs with different parameters:
   ```bash
   cd scripts
   ./run-experiments.sh
   ```

## Analyzing Results

1. After running the simulation:

   ```bash
   python analyze_results.py
   ```

   This will generate:

   - Statistical summaries
   - Visualization plots
   - Performance analysis

## Generated Files

### Data Files

- `manet-results.csv`: Raw simulation data with metrics:
  - Packet Delivery Ratio (PDR)
  - End-to-end delay
  - Hop count
  - Throughput
- `summary_stats.csv`: Statistical summary of results

### Visualizations

- `pdr_analysis.png`: Packet Delivery Ratio analysis
- `delay_cdf.png`: Delay Cumulative Distribution Function
- `hop_distribution.png`: Hop count distribution
- `throughput_analysis.png`: Throughput analysis

## Analysis Results

The complete analysis can be found in `analysis/analisis_manet.txt`. Key findings include:

- Impact of hierarchical mobility on AODV performance
- Comparison of inter-cluster vs intra-cluster communication
- Scalability analysis
- Recommendations for improvement

## Sharing This Work

To share this analysis with others, they will need:

1. This `manet_results` directory containing:

   - Analysis scripts and results
   - Generated data and visualizations
   - Requirements and documentation

2. The `manet.cc` file to run the simulation
   - This should be placed in their NS-3 installation's `scratch` directory
   - They must have NS-3 installed and configured

Note: It is not necessary to share the entire NS-3 installation (ns-3-dev directory).

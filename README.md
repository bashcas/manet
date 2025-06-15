# MANET Performance Analysis

This repository contains the analysis of a hierarchical MANET (Mobile Ad-hoc Network) simulation using NS-3, focusing on the impact of hierarchical mobility on AODV protocol performance.

## Setup

1. Clone this repository:

   ```bash
   git clone https://github.com/your-username/manet-analysis.git
   cd manet-analysis
   ```

2. Clone NS-3 inside the project directory:

   ```bash
   git clone https://gitlab.com/nsnam/ns-3-dev.git
   cd ns-3-dev
   ```

3. Configure and build NS-3:

   ```bash
   ./ns3 configure --enable-examples --enable-tests
   ./ns3 build
   ```

4. Create Python virtual environment:

   ```bash
   cd ..  # back to project root
   python -m venv venv
   source venv/bin/activate  # On Unix/macOS
   # or
   .\venv\Scripts\activate  # On Windows
   ```

5. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

## Directory Structure

```
manet-analysis/
├── manet.cc              # NS-3 simulation code
├── ns-3-dev/             # NS-3 installation (gitignored)
├── data/                 # Simulation results
├── plots/                # Generated visualizations
├── scripts/              # Analysis scripts
└── analysis/            # Documentation
```

## Running the Simulation

1. **Important**: First, copy the simulation file to NS-3's scratch directory:

   ```bash
   cp manet.cc ns-3-dev/scratch/
   ```

2. Run the simulation:

   ```bash
   cd ns-3-dev
   ./ns3 run scratch/manet.cc
   ```

   Optional command line parameters:

   ```bash
   ./ns3 run "scratch/manet.cc --members=5 --time=200 --run=1 --csv=../data/manet-results.csv"
   ```

   Parameters:

   - `--members`: Number of member nodes per cluster (default: 5)
   - `--time`: Simulation duration in seconds (default: 200)
   - `--run`: Run number for this simulation (default: 1)
   - `--csv`: Output CSV file path (default: manet-results.csv)
   - `--append`: Append to existing CSV instead of overwrite (optional)
   - `--tracing`: Enable pcap tracing (optional)

3. For multiple simulation runs:

   ```bash
   cd scripts
   ./run-experiments.sh
   ```

   The script will:

   - Automatically copy manet.cc to ns-3-dev/scratch/
   - Run multiple simulations with different parameters
   - Save results to data/manet-results.csv

   Parameters (configurable in run-experiments.sh):

   - Members per cluster: 3, 5, 7
   - Simulation times: 100s, 200s, 300s
   - 5 runs per configuration

4. Analyze results:
   ```bash
   python analyze_results.py
   ```

## Generated Files

### Data Files

- `data/manet-results.csv`: Raw simulation data with metrics:
  - Packet Delivery Ratio (PDR)
  - End-to-end delay
  - Hop count
  - Throughput
- `data/summary_stats.csv`: Statistical summary of results

### Visualizations

- `plots/pdr_analysis.png`: Packet Delivery Ratio analysis
- `plots/delay_cdf.png`: Delay Cumulative Distribution Function
- `plots/hop_distribution.png`: Hop count distribution
- `plots/throughput_analysis.png`: Throughput analysis

## Analysis Results

The complete analysis can be found in `analysis/analisis_manet.txt`. Key findings include:

- Impact of hierarchical mobility on AODV performance
- Comparison of inter-cluster vs intra-cluster communication
- Scalability analysis
- Recommendations for improvement

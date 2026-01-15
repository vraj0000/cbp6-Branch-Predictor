## BRANCH PREDICTOR RESEARCH AND DESIGN - CBP6 SIMULATOR

1. PROJECT SUMMARY: ALL IMPLEMENTED PREDICTORS
--------------------------------------------------------------------------------
ID  | Predictor Architecture              | Budget (Bits) | MPKI (A-Mean)
----|-------------------------------------|---------------|---------------
#1  | 4k Bimodal                          | 8,192         | 13.3051
#2  | 16k Bimodal                         | 32,768        | 11.0545
#3  | GAg (12-bit Global History)         | 8,204         | 14.1662
#4  | PAg (10-bit Local History)          | 12,288        | 11.2587
#5  | Tournament (Alpha 21264 Style)      | 28,684        | 9.4469
#6  | Perceptron (32-bit GHR)             | 204,824       | 7.5735
#7  | HYBRID NEURAL                       | 471,140       | 6.8008
#8  | O-GEHL (8-Feature Geometric)        | 65,584        | 7.3121
#9  | 3-Feature Table (GHR/PHR/PC)        | N/A           | 10.9173
#10 | TAGE-SC-L (Benchmark)               | 524,288       | 4.1652



2. FLAGSHIP COMPARISON: HYBRID NEURAL (#7) VS. TAGE-SC-L
-------------------------------------------------------
Predictor #7 represents a sophisticated "Neural-Statistical" approach, 
integrating Global, Local, and Path history into a single weighted sum.

Predictor         | IPC (A-Mean) | MR (A-Mean) | MPKI (H-Mean) | CycWPPKI
------------------|--------------|-------------|---------------|-----------------
Hybrid Neural (#7)| 2.7609       | 4.66%       | 1.0274        | 191.3285
TAGE-SC-L (Bench) | 3.1055       | 2.89%       | 1.0337        | 136.3096

3. SCALING & EFFICIENCY METRICS (BITS TO PERFORMANCE)
------------------------------------------------------
"Bits/MPKI" represents the hardware bit cost to reduce the MPKI by 1 unit.

Predictor         | Total Bits | Bits/IPC (Lower=Better) | Bits/MPKI (Lower=Better)
------------------|------------|-------------------------|-------------------------
Bimodal 4k        | 8,192      | 3,975                   | 615
Bimodal 16k       | 32,768     | 14,589                  | 2,964
GAg (12-bit)      | 8,204      | 3,897                   | 579
PAg (10-bit)      | 12,288     | 5,290                   | 1,091
Tournament        | 28,684     | 11,503                  | 3,036
Perceptron (#6)   | 204,824    | 80,000                  | 27,044
Hybrid Neural (#7)| 471,140    | 170,647                 | 69,277
O-GEHL (#8)       | 65,584     | 25,362                  | 8,969
TAGE-SC-L         | 524,288    | 168,825                 | 125,873

Observation: Predictor #7 is significantly more efficient per bit than TAGE-SC-L. 
While TAGE has lower absolute MPKI, the Hybrid Perceptron achieves better 
accuracy density, requiring fewer bits to maintain a sub-1.1 Harmonic Mean MPKI.

4. DETAILED HARDWARE BUDGETS & PERFORMANCE METRICS
--------------------------------------------------

PREDICTOR #1: 4K BIMODAL
- Bit Calculation: 4096 entries * 2 bits
- Metrics: IPC: 2.0607 | MR: 9.24% | MPKI: 13.3051

PREDICTOR #4: PAG (10-BIT HISTORY)
- Bit Calculation: (1024 * 10) Local Hist + (1024 * 2) PHT = 12,288 bits
- Metrics: IPC: 2.3225 | MR: 7.72% | MPKI: 11.2587



PREDICTOR #7: HYBRID NEURAL (GLOBAL + LOCAL + PATH + DYNAMIC THETA)
- Bit Calculation Breakdown:
  * Global Weights: (29 * 8 * 1024) = 237,568 bits
  * Local Weights:  (10 * 8 * 1024) = 81,920 bits
  * Path Weights:   (16 * 8 * 1024) = 131,072 bits
  * Local BHT:      (1024 * 10)      = 10,240 bits
  * Logic/Indices:  (Misc)           = 10,340 bits
- TOTAL BITS: 471,140 bits (~57.5 KB)
- Metrics: IPC: 2.7609 | MR: 4.66% | MPKI: 6.8008 | CycWPPKI: 191.3285

PREDICTOR #10: TAGE-SC-L (BENCHMARK)
- Bit Calculation: 524,288 bits (64 KB Limit)
- Metrics: IPC: 3.1055 | MR: 2.89% | MPKI: 4.1652 | CycWPPKI: 136.3096

5. INDEPENDENT STUDY & REFERENCES
----------------------------------
This research was conducted independently by following the Prof. Onur Mutlu 
(ETH Zurich) Digital Design and Computer Architecture lecture series.

[1] Seznec, A. "Genesis of the O-GEHL branch predictor." JILP, 2005.
[2] Jiménez, D. A. "Multiperspective Perceptron Predictor." CBP-6, 2019.
    (Inspiration for the 3-Feature Predictor #9).
[3] Jiménez, D. A., & Lin, C. "Dynamic Branch Prediction with Perceptrons." 
    HPCA, 2001.
[4] Jimenez, D. A. "Piecewise Linear Branch Prediction." ISCA, 2005.
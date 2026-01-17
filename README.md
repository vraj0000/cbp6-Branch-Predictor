## BRANCH PREDICTOR RESEARCH AND DESIGN - CBP6 SIMULATOR

### 1. PROJECT SUMMARY: ALL IMPLEMENTED PREDICTORS
--------------------------------------------------------------------------------
ID  | Predictor Architecture              | Budget (Bits) | MPKI (A-Mean)
----|-------------------------------------|---------------|---------------
1   | 4k Bimodal                          | 8,192         | 13.3051
2   | 16k Bimodal                         | 32,768        | 11.0545
3   | GAg (12-bit Global History)         | 8,204         | 14.1662
4   | PAg (10-bit Local History)          | 12,288        | 11.2587
5   | Tournament (Alpha 21264 Style)      | 28,684        | 9.4469
6   | Perceptron (32-bit GHR)             | 204,824       | 7.5735
7   | HYBRID NEURAL                       | 471,140       | 6.8008
8   | O-GEHL (8-Feature Geometric)        | 65,584        | 7.3121
9   | 3-Feature Table (GHR/PHR/PC)        | N/A           | 10.9173
10  | TAGE-SC-L (Benchmark)               | 524,288       | 4.1652


### 2. PREDICTOR ANALYSIS

#### A: Bimodal
Simple and holds its ground but is limited by exponential power requirements. Mapping the PC to a small table causes significant aliasing.

#### B: GAg (Global History Register, Global History Table)
Looking at the IPC and MPKI, it performs sub-par compared to Bimodal. However, it is efficient: using 74.97% less area, it achieves 93% of the accuracy. While not beating Bimodal, it represents a strong area-vs-accuracy trade-off.

#### C: PAg (Per-Address History Register, Global History Table)
PAg combines history recording per address, reducing aliasing in the pattern history table. It excels in programs where branches are tightly coupled. By recording history using 62.5% less area and achieving a competitive MPKI, it captures hard-to-predict branches more effectively than Bimodal, resulting in a higher IPC.

#### D: Tournament
This is an Alpha 21264-style tournament predictor, combining the features of both PAg and GAg with a choice table to decide between them. I experimented with a 24-bit GHR folded into 12 bits for the GAg component, thinking it would encode more information; however, IPC decreased by 0.01 and MPKI increased by 0.93.

#### E: Perceptron
The Perceptron uses a 32-bit GHR and a 10-bit address. This is a "heavy hitter" in terms of memory. Out of curiosity, I tested a pure 24-bit GAg, but the IPC did not approach 2.2 and it was completely unrealizable in hardware. The Perceptron is large but represents a significant step toward TAGE-SC-L performance.

#### F: Hybrid Neural
The name comes from the combination of inputs it utilizes. It features a 28-bit GHR with a bias weight, a 16-bit Path History (PHR), and 10-bit Local History. One weight is formed from a PAg (10-bit PC XORed with GHR), and a 12-bit GAg contributes another weight (taking the GHR from bit 29 onwards). The feature vector is: **{28b GHR + 16b PHR + 10b Local + 1b PAg + 1b GAg}**.

This reached 2.7609 IPC. I also created a "behemoth" version with 32-bit GHR, same PHR/Local/PAg, but up to 128 bits of global history with 16 segments of GAg feeding into the perceptron. This reached 2.9102 IPC. While highly impractical for hardware, it was an attempt to incorporate longer history into a neural framework. The goal of adding PAg and GAg was to provide non-linear inputs to the perceptron, allowing the linear matcher to make better decisions based on non-linear data.

#### G: O-GEHL
This model was a revelation. Previously, I viewed features as vectors to be integrated into a perceptron; here, the feature is inherent to the index, and weights are extracted from tables. We use fewer tables than a standard perceptron, but the features map directly to weights. Training follows the perceptron rule, but the threshold (Theta) is adjusted on the fly. It uses only 8 tables but can be extended linearly; history length increases geometrically, utilizing drastically fewer weights.

### 3. REMARKS
As we move from Bimodal to O-GEHL, predictors become significantly "smarter." TAGE combines the geometric lengths of O-GEHL with a mapping system that handles non-linear functions like XOR. My Hybrid Neural was inspired by the Multi-Perspective Perceptron (MPP) predictor, which looks at address, path, global/local history, and stacks. Combing non-linear mapping with perceptron logic allows the model to better analyze what is coming, what is happening, and what has happened to make superior predictions.

### 4. FLAGSHIP COMPARISON: HYBRID NEURAL (#7) VS. TAGE-SC-L
-------------------------------------------------------
Predictor #7 represents a sophisticated "Neural-Statistical" approach, integrating Global, Local, and Path history into a single weighted sum.

Predictor         | IPC (A-Mean) | MR (A-Mean) | MPKI (H-Mean) | CycWPPKI
------------------|--------------|-------------|---------------|-----------------
Hybrid Neural (#7)| 2.7609       | 4.66%       | 1.0274        | 191.3285
TAGE-SC-L (Bench) | 3.1055       | 2.89%       | 1.0337        | 136.3096

### 5. SCALING & EFFICIENCY METRICS
------------------------------------------------------
Predictor         | Total Bits | IPC                     | MPKI        |  CycWPPKI    
------------------|------------|-------------------------|-------------|-----------
Bimodal 4k        | 8,192      | 2.0607                  | 13.3051     |  322.4358
Bimodal 16k       | 32,768     | 2.2459                  | 11.0545     |  281.3167
GAg (12-bit)      | 8,204      | 2.1051                  | 14.1662     |  336.1540
PAg (10-bit)      | 12,288     | 2.3225                  | 11.2587     |  278.1750
Tournament        | 28,684     | 2.4376                  | 9.4469      |  259.6916
Perceptron        | 270,336    | 2.6853                  | 7.5755      |  206.5263
Hybrid Neural (#7)| 471,140    | 2.7609                  | 6.8008      |  191.3285
O-GEHL (#8)       | 65,584     | 2.5859                  | 7.3121      |  210.3925
TAGE-SC-L         | 524,288    | 3.1055                  | 4.1652      |  136.3096

### 6. INDEPENDENT STUDY & REFERENCES
----------------------------------
This research was conducted independently following the Prof. Onur Mutlu (ETH Zurich) Digital Design and Computer Architecture lecture series.

[1] Seznec, A. "Genesis of the O-GEHL branch predictor." JILP, 2005.

[2] Jiménez, D. A. "Multiperspective Perceptron Predictor." CBP-6, 2019.

[3] Jiménez, D. A., & Lin, C. "Dynamic Branch Prediction with Perceptrons." HPCA, 2001.

[4] Jimenez, D. A. "Piecewise Linear Branch Prediction." ISCA, 2005.

[5] Kessler, R. E. "The Alpha 21264 Microprocessor." IEEE Micro, 1999.



I’ve spent the last few weeks diving into high-performance branch prediction. What started with Professor Onur Mutlu’s (@Onur Mutlu) Fundamentals of Computer Architecture CBP-6 simulator.

In the final lab task—an open-budget challenge—I pushed the limits by building a "Behemoth" Predictor: 12 GAg segments and 1 PAg feeding into a Perceptron with a 28-bit GHR, 10-bit Local History, and 16-bit Path History. This experimental setup hit an IPC of 2.901, closing in on the benchmark TAGE-SC-L (3.10 IPC).

Project Highlights:

Evolution of Complexity: I implemented 10 predictors, scaling from simple 4k Bimodal tables to a Hybrid Neural model. Using a custom feature vector, I reached 2.76 IPC—a significant jump, though the logic depth makes it a challenge for real-world hardware timing.

O-GEHL Efficiency: This was the "aha!" moment. I implemented an 8-table geometric history length predictor that achieved 7.31 MPKI using only 65k bits. It is 4.1x more area-efficient than a standard 270k-bit Perceptron.

The Non-Linear Edge: My Hybrid Neural design was inspired by the Multiperspective Perceptron (MPP). By feeding non-linear inputs (PAg/GAg) into a linear matcher, the model learns which history "expert" to trust for specific branch patterns.

Key Insight: Architecture is the art of trade-offs. While long histories catch that "last stretch" of hard-to-predict branches, the real engineering challenge is managing adder-tree latency and bit budgets.

The journey doesn't stop!

#ComputerArchitecture #HardwareEngineering #CPUDesign #OnurMutlu #CBP6 #NeuralNetworks #DigitalDesign #OpenSourceEducation
# CBP-2025 Simulator
This simulator is publicly distributed for [6th Championship Branch Prediction (CBP), 2025](https://ericrotenberg.wordpress.ncsu.edu/cbp2025/). We refactored the code to make it suitable to be used in the lab of Fundamentals of Computer Architecture (FoCA) course in ETH Zürich. The original CBP-2025 simulator repository can be found at: https://github.com/ramisheikh/cbp2025 

## Brief Code Walkthrough
The simulator expects a branch predictor to be implemented in the files `my_pred.h` and `my_pred.cc`. The predictor gets statically instantiated. For reference, check sample predictor files: [my_pred.h](./my_pred.h) and [my_pred.cc](./my_pred.cc)

### Branch Predictor Interface

The branch predictor interacts with the rest of the simulator via the following interfaces:
* `beginCondDirPredictor()` - Intended for any predictor initialization steps.
* `get_cond_dir_prediction()` - invoke the predictor to get the prediction of the relevant branch. This is called only for conditional branches.
* `spec_update()` - Intended to help update the predictor's history (GHR/LHIST ..etc.) This is called for all branches right after a prediction is made.
* `notify_instr_decode()` - Called when an instruction is decoded.
* `notify_instr_execute_resolve()` - Called when any instruction is executed.
* `notify_instr_commit()` - Called when any instruction is committed.
* `endCondDirPredictor()` - Called at the end of simulation to allow contestants to dump any additional state.

These interfaces get exercised as the instruction flows through the cpu pipeline, and they provide the contestants with the relevant state available at that pipeline stage. The interfaces are defined in [cbp.h](./cbp.h) and must remain unchanged. The structures exposed via the interfaces are defined in [sim_common_structs.h](lib/sim_common_structs.h). This includes InstClass, DecodeInfo, ExecuteInfo ..etc.

See [cbp.h](./cbp.h) and [cond_branch_predictor_interface.cc](./cond_branch_predictor_interface.cc) for more details.

<!-- ### Contestant Developed Predictor

The simulator comes with CBP2016 winner([64KB Tage-SC-L](./cbp2016_tage_sc_l.h)) as the conditional branch predictor. Contestants may retain the Tage-SC-L and add upto 128KB of additional prediction components, or discard it and use the entire 192KB for their own components. Contestants are also allowed to update tage-sc-l implementation.
Contestants are free to update the implementation within [cond_branch_predictor_interface.cc](./cond_branch_predictor_interface.cc) as long as they keep the branch predictor interfaces (listed above) untouched. E.g., they can modify the file to combine the predictions from the cbp2016 tage-sc-l and their own developed predictor.

In a processor, it is typical to have a structure that records prediction-time information that can be used later to update the predictor once the branch resolves. In the provided Tage-SC-L implementation, the predictor checkpoints history in an STL map(pred_time_histories) indexed by instruction id to serve this purpose. At update time, the same information is retrieved to update the predictor.
For the predictors developed by the contestants, they are free to use a similar approach. The amount of state needed to checkpoint histories will NOT be counted towards the predictor budget. For any questions, contestants are encouraged to email the CBP2025 Organizing Committee. -->

## How to Use
0. Build the simulator using:

    ```
    make clean && make
    ```
1. To see simulator options:

    ```bash
    ./cbp
    ```

2. To run the simulator with `trace.gz` trace:

    ```bash
    ./cbp trace.gz
    ```

3. To run the simulator for 10M instructions with a periodic print for completing every 100K instructions:

    ```bash
    ./cbp -S 10000000 -H 100000 trace.gz
    ```

## Key Points to Note

1. Run `make clean && make` to ensure your changes are taken into account.

2. Sample traces are provided : [sample_traces](./sample_traces)

3. A sample script to run all traces and dump a csv is also provided : [trace_exec_training_list](scripts/trace_exec_training_list.py)

4. A sample reference result from the training set are included here : [reference_results](reference_results_training_set.csv)

5. To run the script, update the trace_folder and results dir inside the script and run the following command. The script executes all the traces inside the trace directory and creates a directory structure with the logs similar to thr trace-directory with all the logs. The script also parses all the logs to dump a csv with relevant stats.
    
    ```bash
    python trace_exec_training_list.py  --trace_dir sample_traces/ --results_dir  sample_results
    ```

## Getting Traces

Download the trace from this git repository: https://gitlab.ethz.ch/rahbera/cbp6_traces. This repository contains 40 traces. You will only use these traces for the scope of this lab.

However, if you are interested to test with even more traces, you can find a list of 105 traces that are provided by the CBP-2025 championship here: [Google Drive](https://drive.google.com/drive/folders/10CL13RGDW3zn-Dx7L0ineRvl7EpRsZDW)

You can use 'gdown' do download the traces:

```bash
pip install gdown
gdown --folder //drive.google.com/drive/folders/10CL13RGDW3zn-Dx7L0ineRvl7EpRsZDW
tar -xvf foo.tar.xz
```

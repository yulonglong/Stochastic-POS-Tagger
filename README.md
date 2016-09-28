Part-of-speech Tagger
================================
A Stochastic (HMM)  POS bigram tagger was developed in C++ using Penn Treebank tag set.  
Viterbi algorithm which runs in O(T.N²) was implemented to find the optimal sequence of the most probable tags.  
*T = number of words ; N = number of POS tags.*  

**Requirements:**  
- C++ compiler (i.e., `g++`) is required.  
- To check whether `g++` is installed in your computer, go to terminal or command prompt and type `g++`.  
- If the command is not found, it means `g++` is not installed in your computer or `g++` has not been included in your `PATH`.  

**I don't want any details. How do I compile and run it?**

1. Go to (i.e., `cd`) the root directory of this repository.
    - `~/github/Stochastic-POS-Tagger/ $>`  
2. Run the demo script from terminal or command prompt.
    - UNIX : `$> ./demoUNIX.sh`
    - Windows : `$> demoWINDOWS.bat`
3. Done! Models and result files are in the same root directory.

**Training, development, and test file:**

1. `sents.train`  
    - Training file  
2. `sents.devt`  
    - Development file  
3. `sents.test`  
    - Test file  
    
**Source files:**

1. `build_tagger.cpp`:  
    - Compilation command: `g++ -Wall build_tagger.cpp -o build_tagger.exe`  
    - Execution command: `build_tagger.exe <training_filename> <development_filename> <model_filename>`  
      - e.g. `build_tagger.exe sents.train sents.devt model_file`  
    - POS tagger will read `sents.train` and extract relevant information required  
    - It will create a Model, and save it into a text file `model_file`  
    - It will also create `model_statistics.txt` for the summary of the training.  
2. `run_tagger.cpp`:  
    - Compilation command: `g++ run_tagger.cpp -o run_tagger.exe`  
    - Execution command : `run_tagger.exe <test_filename> <model_filename> <output_filename>`  
      - e.g. `run_tagger.exe sents.test model_file sents.out`  
    - POS tagger will read the necessary model information from `model_file`  
    - It will read `sents.test`, run Viterbi algorithm, and output the words with their corresponding most probable (predicted) POS tags in `sents.out`  
3. `Storage.cpp`:  
    - Compilation is not required for `Storage.cpp`  
    - The class with the appropriate data structure to contain the relevant attributes and information of the model, stored in `model_file` as mentioned above  
echo '==== Compiling Source Code... ===='
g++ -Wall source/build_tagger.cpp -o build_tagger.exe
g++ -Wall source/run_tagger.cpp -o run_tagger.exe
echo '==== Compilation done. ===='

echo '==== Building Model...  ===='
build_tagger.exe data/sents.train data/sents.devt model_file
echo '==== Model has been built successfully.  ===='

echo '==== Running test...  ===='
run_tagger.exe data/sents.test model_file sents.out
echo '==== Test done.  ===='
echo '==== The POS-tagged sentences are in sents.out.  ===='

#!/bin/bash

echo "Installing Emscripten SDK..."

git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
chmod 777 ./emsdk_env.sh
source ./emsdk_env.sh
emcc -v
cd ..

echo "Compiling C++ project..."

#get all cpp files in src folder 
srcFiles=(\
./src/tinyLayoutEngine.cpp \
./src/tinyLayoutEngineWasm.cpp \
)

#make obj build dir if it doesnt exist
mkdir -p ./wasmdist/obj

#compile to obj files
objList=""
for i in "${srcFiles[@]}"
do
    fileName=$(basename -- "$i") 
    objList="${objList} ./wasmdist/obj/${fileName}.o"

    echo building "${fileName}.o"

    echo building...
    if [ "$1" == "prod" ]
    then
        emcc -c -std=c++17 -O3 -o ./wasmdist/obj/$fileName.o $i &
    else
        emcc -c -std=c++17 -g -gsource-map -o ./wasmdist/obj/$fileName.o $i &
    fi

done

wait 

#run linking 
echo linking...
if [ "$1" == "prod" ]
then
    emcc -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s ENVIRONMENT='web' -s EXPORT_ES6=1 -s --bind -std=c++17 -O3 -o ./wasmdist/tinyLayoutEngine.mjs $objList
else
    emcc -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s ENVIRONMENT='web' -s EXPORT_ES6=1 -s ASSERTIONS=1 --bind -std=c++17 -g -gsource-map -o ./wasmdist/tinyLayoutEngine.mjs $objList
fi

#copy wasm file to test build folder 
echo copy wasm module to test folder...
mkdir -p ./wasmtest/build/
cp ./wasmdist/tinyLayoutEngine.wasm ./wasmtest/build/
if [ "$1" != "prod" ]
then
    cp ./wasmdist/tinyLayoutEngine.wasm.map ./wasmtest/build/
fi
cp ./wasmdist/tinyLayoutEngine.mjs ./wasmtest/build/

sudo apt update
sudo apt install qt6-base-dev qt6-declarative-dev qt6-svg-dev qt6-base-private-dev qt6-declarative-private-dev qt6-tools-dev qt6-l10n-tools qt6-languageserver-dev
rm -rf build/
mkdir ./build
cd ./build
cmake ..
cmake --build . --target chronicle_lupdate # updates the .ts files in the source directory
make -j4

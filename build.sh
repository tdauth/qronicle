sudo apt update
sudo apt install qt6-base-dev qt6-declarative-dev qt6-svg-dev qt6-base-private-dev qt6-declarative-private-dev
rm -rf build/
mkdir ./build
cd ./build
cmake ..
make -j4

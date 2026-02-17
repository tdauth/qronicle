# Ubuntu dependencies
sudo apt update
sudo apt install \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-svg-dev \
  qt6-base-private-dev \
  qt6-declarative-private-dev \
  libqt6quickwindow6-dev \
  qt6-tools-dev \
  qt6-l10n-tools \
  qt6-languageserver-dev \
  qml-qt6 \
  qml6-module-qtquick-templateses \
  qml6-module-qtquick-window

# Clean build
rm -rf ../build/
mkdir ../build
cd ../build
#cmake ..
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target qronicle_lupdate # updates the .ts files in the source directory
cmake --build . -j $(nproc)

# .DEB package for Ubuntu
cpack
sudo dpkg -i qronicle-1.0.0-Linux.deb
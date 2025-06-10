CXX = g++
CXXFLAGS = -std=c++17 -Wall
LIBS = -lfltk -lfltk_images -ljpeg -lpng

catalogo: main.cpp
	$(CXX) $(CXXFLAGS) -o catalogo main.cpp $(LIBS)

clean:
	rm -f catalogo

install_deps_ubuntu:
	sudo apt-get update
	sudo apt-get install libfltk1.3-dev libjpeg-dev libpng-dev

install_deps_fedora:
	sudo dnf install fltk-devel libjpeg-devel libpng-devel

install_deps_arch:
	sudo pacman -S fltk libjpeg libpng

.PHONY: clean install_deps_ubuntu install_deps_fedora install_deps_arch
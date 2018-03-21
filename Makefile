all: 
	@echo "make shared && sudo make install"
	@echo "make static && sudo make install"
install:
	cd build && make install
static: 
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF 
	cmake --build ./build 
shared: 
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
	cmake --build ./build 

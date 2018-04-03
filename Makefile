all: 
	@echo "Usage:"
	@echo "	make shared"
	@echo "	make static"
install:
	cd build && make install
doc: 
	if hash doxygen 2>/dev/null; then doxygen; fi
	mkdir -p build
static: doc
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF 
	cmake --build ./build 
shared: doc
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
	cmake --build ./build 


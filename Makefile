help:
	@echo "Usage:"
	@echo "	make help"
	@echo "	make shared"
	@echo "	make static"
	@echo "	make all"
	@echo "	make light"

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
all: 	doc
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=ON 
	cmake --build ./build 
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
	cmake --build ./build 
light:
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DWFLOW_DISABLE_JSON=ON -DWFLOW_DISABLE_LOG=ON
	cmake --build ./build 
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DWFLOW_DISABLE_JSON=ON -DWFLOW_DISABLE_LOG=ON
	cmake --build ./build 
	


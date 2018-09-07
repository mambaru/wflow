help:
	@echo "Usage:"
	@echo "	make help"
	@echo "	make shared"
	@echo "	make static"
	@echo "	make all"
	@echo "	make doc"
	@echo "	make clean"
	@echo "	make install"

install:
	cd build && make install
doc:
	rm -rf ./docs
	if hash doxygen 2>/dev/null; then doxygen; fi
static: 
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF 
	cd build && cmake --build .
shared: 
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
	cd build && cmake --build .
all: 	
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=ON 
	cd build && cmake --build .
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
	cd build && cmake --build .
clean:
	rm -r docs
	cd build && make clean

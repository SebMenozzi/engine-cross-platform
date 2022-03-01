.PHONY: release debug clean run

debug:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=DEBUG ..
	cd build; make -j8

release:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=RELEASE ..
	cd build; make -j8

xcode:
	cmake -S . -B ./build/MacOS -G "Xcode"

clean:
	rm -rf build

run:
	./build/engine

%:
	@:
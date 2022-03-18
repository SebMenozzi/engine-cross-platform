.PHONY: xcode debug clean

xcode:
	cmake -S . -B build/MacOS -G Xcode
	open build/MacOS/engine.xcodeproj

debug:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=DEBUG ..
	cd build; make -j8

clean:
	rm -rf build

%:
	@:
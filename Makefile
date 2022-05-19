.PHONY: xcode-macos xcode-ios desktop clean

xcode-macos:
	cmake -S . -B build/MacOS -G Xcode
	open build/MacOS/engine.xcodeproj

xcode-ios:
	cmake -S . -B build/iOS -G Xcode
	open build/iOS/engine.xcodeproj

desktop:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=DEBUG ..
	cd build; make -j8
	./build/desktop/desktop

clean:
	rm -rf build
.PHONY: xcode xcode-ios debug clean

xcode:
	cmake -S . -B build -G Xcode
	open build/engine.xcodeproj

xcode-ios:
	cmake -S . -B build -G Xcode -D CMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake -D IOS_PLATFORM=OS
	open build/engine.xcodeproj

debug:
	cmake -S . -B build -D CMAKE_BUILD_TYPE=DEBUG
	cd build; make -j8
	./build/desktop/desktop

clean:
	rm -rf build
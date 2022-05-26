.PHONY: xcode xcode-ios desktop clean

xcode:
	make clean;
	cmake -S . -B build -G Xcode -D CMAKE_IOS_APP=OFF
	open build/engine.xcodeproj

xcode-ios:
	make clean;
	cmake -S . -B build -G Xcode -D CMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake -D IOS_PLATFORM=OS -D CMAKE_IOS_APP=ON
	open build/engine.xcodeproj

debug:
	make clean;
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=DEBUG ..
	cd build; make -j8

desktop:
	./build/desktop/desktop

iosapp:
	open ./app/iOS/iosapp.xcodeproj

clean:
	rm -rf build
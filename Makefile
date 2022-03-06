.PHONY: xcode clean

xcode:
	cmake -S . -B build/MacOS -G Xcode
	open build/MacOS/engine.xcodeproj

clean:
	rm -rf build

%:
	@:
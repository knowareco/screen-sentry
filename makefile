all: build

build:
	@echo "Building..."
	pio run

upload:
	@echo "Uploading..."
	pio run -t upload

flash: upload

monitor:
	@echo "Starting Monitor..."
	pio device monitor

clean:
	@echo "Cleaning..."
	pio run -t clean

menuconfig:
	@echo "Running menuconfig..."
	pio run -t menuconfig

compiledb:
	@echo "Generating compile_commands.json..."
	pio run -t compiledb
	ln -sf .pio/build/arduino_nano_esp32/compile_commands.json compile_commands.json

debug:
	@echo "Start Debugging..."
	$(MAKE) build
	$(MAKE) upload
	$(MAKE) monitor

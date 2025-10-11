BUILD_DIR = build
APP_NAME = "SoundStuff"
APP_PATH = $(BUILD_DIR)/SoundStuff_artefacts/$(APP_NAME).app

# default target
.PHONY: all
all: build

# create build directory and run cmake
.PHONY: configure
configure:
	@echo "Configuring project..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..

# build the project
.PHONY: build
build: configure
	@echo "Building project..."
	@cd $(BUILD_DIR) && make

# run the app
.PHONY: run
run: build
	@echo "Running $(APP_NAME)..."
	@open "$(APP_PATH)"

# clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# full clean (including JUCE cache)
.PHONY: distclean
distclean: clean
	@echo "Deep cleaning (removing JUCE cache)..."
	@rm -rf _deps

# rebuild from scratch
.PHONY: rebuild
rebuild: clean build

# quick build (skip configure if build dir exists)
.PHONY: quick
quick:
	@echo "Quick build..."
	@if [ -d "$(BUILD_DIR)" ]; then \
		cd $(BUILD_DIR) && make; \
	else \
		$(MAKE) build; \
	fi

# show help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  build      - Configure and build the project"
	@echo "  run        - Build and run the application"
	@echo "  clean      - Remove build directory"
	@echo "  distclean  - Remove build directory and JUCE cache"
	@echo "  rebuild    - Clean and build from scratch"
	@echo "  quick      - Fast build (skip configure)"
	@echo "  help       - Show this help message"

.PHONY: configure build run clean distclean rebuild quick help all
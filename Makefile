
BUILD_DIR = bld
SOURCE_DIR = .

CMAKE = cmake
GODO = $(CMAKE) -E chdir

CMAKE_BUILD_TYPE = RelOnHost
CMAKE_BUILD_OPTIONS = -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
ifdef GGML_BLAS
CMAKE_BUILD_OPTIONS += -DGGML_BLAS:BOOL=$(GGML_BLAS)
endif
ifdef GGML_OPENMP
CMAKE_BUILD_OPTIONS += -DGGML_OPENMP:BOOL=$(GGML_OPENMP)
endif
ifdef GGML_VULKAN
CMAKE_BUILD_OPTIONS += -DGGML_VULKAN:BOOL=$(GGML_VULKAN)
endif


.PHONY: default all cmake proj \
	test clean distclean \
	update pull

default:
	if [ ! -d $(BUILD_DIR) ] ; then $(MAKE) cmake ; fi
	$(MAKE) proj

all:
	$(MAKE) cmake
	$(MAKE) proj

cmake:
	$(CMAKE) $(CMAKE_BUILD_OPTIONS) -S $(SOURCE_DIR) -B $(BUILD_DIR)

proj:
	$(GODO) $(BUILD_DIR) $(MAKE)

test:
	$(GODO) $(BUILD_DIR) $(MAKE) test

clean:
	$(GODO) $(BUILD_DIR) $(MAKE) clean

distclean:
	rm -fr $(BUILD_DIR)

update:
	git pull origin trunk

pull:
	git pull origin trunk


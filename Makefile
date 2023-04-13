
BldPath=bld

CMakeExe=cmake
CMAKE=$(CMakeExe)
GODO=$(CMakeExe) -E chdir
MKDIR=$(CMakeExe) -E make_directory

.PHONY: default all cmake proj \
	test clean distclean \
	update pull

default:
	if [ ! -d $(BldPath) ] ; then $(MAKE) cmake ; fi
	$(MAKE) proj

all:
	$(MAKE) cmake
	$(MAKE) proj

cmake:
	if [ ! -d $(BldPath) ] ; then $(MKDIR) $(BldPath) ; fi
	#$(GODO) $(BldPath) $(CMAKE) -D CMAKE_BUILD_TYPE=RelOnHost -D LLAMA_OPENBLAS=ON ..
	$(GODO) $(BldPath) $(CMAKE) -D CMAKE_BUILD_TYPE=RelOnHost ..

proj:
	$(GODO) $(BldPath) $(MAKE)

test:
	$(GODO) $(BldPath) $(MAKE) test

clean:
	$(GODO) $(BldPath) $(MAKE) clean

distclean:
	rm -fr $(BldPath)

update:
	git pull origin trunk

pull:
	git pull origin trunk


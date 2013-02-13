
all: material_scanner.cpp material_parser.h material_parser.cpp

material_scanner.cpp: material_scanner.l material_parser.h
	flex -omaterial_scanner.cpp material_scanner.l
	
material_parser.h material_parser.cpp: material_parser.y
	bison --defines=material_parser.h -omaterial_parser.cpp material_parser.y
	
.PHONY: clean
clean:
	-rm material_scanner.cpp material_parser.h material_parser.cpp location.hh position.hh stack.hh
	
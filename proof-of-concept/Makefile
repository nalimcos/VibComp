all:	edit vibcomp_test.gcode

edit:
	nano vibcomp_test.cpp

vibcomp_test.gcode:	clean vibcomp_test
	./vibcomp_test
	@echo "=== vibcomp_test.gcode ==="
	@cat vibcomp_test.gcode
	@echo "====== eof ======"

vibcomp_test:
	g++ vibcomp_test.cpp -o vibcomp_test -lm

clean:
	rm -f vibcomp_test vibcomp_test.gcode

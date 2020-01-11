all:	edit test

edit:
	nano vib_sim.cpp

test:	clean vib_sim
	./vib_sim
	@echo "=== output.ps ==="
	@cat output.ps
	@echo "====== eof ======"

vib_sim:
	g++ vib_sim.cpp -o vib_sim -lm

clean:
	rm -f vib_sim output.ps

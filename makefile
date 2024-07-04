
all: main

main: main.cpp kosaraju_vec_edges.hpp kosaraju_adj_mat.hpp reactor.hpp proactor.hpp
	g++ -pg -o main main.cpp
	
clean:
	rm -f main gmon.out

# Make file for llvm-opencl project 

compiler = clang++
flag = -std=c++11 -g `llvm-config --cxxflags --ldflags --libs all`
src_path = ./src
result_path = ./results
tb_path = ./tb

.PHONY : canyon clean

canyon :
	$(compiler) -shared $(flag) ./src/canyon.cpp $(src_path)/DDG.cpp -o ./results/canyon.so

clean : 
	rm $(result_path)/*.so

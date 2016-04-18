# Make file for llvm-opencl project 

compiler = clang++
flag = -std=c++11 -g `llvm-config --cxxflags --ldflags --libs all`
src_path = ./src
result_path = ./results
tb_path = ./tb

main : 
	$(compiler) $(src_path)/main.cpp $(src_path)/DDG.cpp $(flag) -o $(result_path)/main.exe

.PHONY : gcd.ll loop.ll gcd.bc gcd.dot loop.dot vadd mm dist pass canyon clean

gcd.ll :
	clang -O3 -emit-llvm $(tb_path)/gcd.c -S -o $(result_path)/gcd.ll

loop.ll :
	clang -O2 -emit-llvm $(tb_path)/loop.c -S -o $(result_path)/loop.ll

gcd.bc :
	clang -O3 -emit-llvm $(tb_path)/gcd.c -c -o $(result_path)/gcd.bc

gcd.dot :
	opt -dot-cfg $(result_path)/gcd.ll

loop.dot :
	opt -dot-cfg $(result_path)/loop.ll

vadd :
	clang -S -emit-llvm -o $(result_path)/vadd.ll -x cl $(tb_path)/vadd/vadd.cl
	opt -dot-cfg $(result_path)/vadd.ll

mm :
	clang -S -emit-llvm -o $(result_path)/mm.ll -x cl $(tb_path)/mm/mm.cl
	opt -dot-cfg $(result_path)/mm.ll

dist :
	clang -S -emit-llvm -o $(result_path)/dist.ll -x cl $(tb_path)/dist/dist.cl
	opt -dot-cfg $(result_path)/dist.ll

pass :
	$(compiler) -shared $(flag) ./src/pass.cpp -g -o ./results/pass.so

canyon :
	$(compiler) -shared $(flag) ./src/canyon.cpp $(src_path)/DDG.cpp -o ./results/canyon.so

clean : 
	rm $(result_path)/main.exe

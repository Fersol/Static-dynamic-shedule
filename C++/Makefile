
all:
	g++  -std=c++14 mainh.cpp process_heterogenes.cpp types.cpp -o shedulerH

stest:
	./shedulerH 1 TestsHeterogenes/STEST/task Systems/system2 synthesis

shtest:
	./shedulerH 1 TestsHeterogenes/STEST/task Systems/system2 schedule

prod_sh_test:
	./shedulerH 1 TestsHeterogenes/test_0.csv Systems/system2 schedule
prod_sh_test1:
	./shedulerH 1 TestsHeterogenes/test_1.csv Systems/system2 schedule
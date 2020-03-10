
all:
	g++  -std=c++14 mainh.cpp process_heterogenes.cpp types.cpp -o shedulerH

zerotest:
	./shedulerH 1 TestsHeterogenes/TEST/task TestsHeterogenes/TEST/system schedule

badtest:
	./shedulerH 1 TestsHeterogenes/TEST1/task TestsHeterogenes/TEST1/system schedule

test:
	./shedulerH 1 TestsHeterogenes/TEST0/task TestsHeterogenes/TEST0/system schedule

stest:
	./shedulerH 1 TestsHeterogenes/STEST/task TestsHeterogenes/STEST/system synthesis

shtest:
	./shedulerH 1 TestsHeterogenes/STEST/task TestsHeterogenes/STEST/system schedule

prod_sh_test:
	./shedulerH 1 TestsHeterogenes/test_0.csv TestsHeterogenes/system_0.csv schedule
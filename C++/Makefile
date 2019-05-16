all:
	g++ main.cpp process.cpp types.cpp -o sheduler

h:
	g++ mainh.cpp process_heterogenes.cpp types.cpp -o shedulerH

testh1:
	shedulerH 1 TestsHeterogenes/test1 Systems/system1

testh2:
	shedulerH 1 TestsHeterogenes/test2 Systems/system1

testh:
	shedulerH 1 TestsHeterogenes/TEST0/task TestsHeterogenes/TEST0/system
	shedulerH 1 TestsHeterogenes/TEST1/task TestsHeterogenes/TEST1/system

test1:
	sheduler 1 1 Tests/test1

test2:
	sheduler 1 1 Tests/test2
	sheduler 1 1 Tests/test3
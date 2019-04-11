all:
	g++ main.cpp process.cpp types.cpp -o sheduler

h:
	g++ mainh.cpp process_heterogenes.cpp types.cpp -o shedulerH

testh1:
	shedulerH 1 TestsHeterogenes/test1 Systems/system1

testh2:
	shedulerH 1 TestsHeterogenes/test2 Systems/system1

test1:
	sheduler 1 1 Tests/test1

test2:
	sheduler 1 1 Tests/test2
	sheduler 1 1 Tests/test3
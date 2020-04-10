
all:
	g++  -std=c++14 src/mainh.cpp src/process_heterogenes.cpp src/types.cpp -o scheduler

test_syn:
	./scheduler 1 tasks/STEST/task systems/system2 synthesis

test_sch:
	./scheduler 1 tasks/STEST/task systems/system2 schedule

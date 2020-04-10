
all:
	g++  -std=c++14 src/main.cpp src/process_heterogenes.cpp src/types.cpp -o scheduler

test_syn:
	./scheduler 1 tasks/test_task.tsv systems/system2 synthesis

test_sch:
	./scheduler 1 tasks/test_task.tsv systems/system2 schedule

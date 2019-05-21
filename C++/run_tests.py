import os

command_template = "shedulerH 1 TestsHeterogenes/TEST{i}/task TestsHeterogenes/TEST{i}/system"

for i in range(192):
    os.system(command_template.format(i=i))
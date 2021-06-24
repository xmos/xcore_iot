# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import matplotlib.pyplot as plt

class tile_grapher:
    def __init__(self, records_list, tileno):
        self.records_list = records_list
        self.tileno = tileno

    def graph(self):
        task_name_list = []
        last_rec_time = 0;
        for core in self.records_list:
            for rec in core:
                if rec["name"] not in task_name_list:
                    task_name_list.append(rec["name"])
                if int(rec["rectime"]) > last_rec_time:
                    last_rec_time = int(rec["rectime"])

        # print(self.task_name_list)
        # print(last_rec_time)

        num_tasks = len(task_name_list)

        cmap = plt.cm.rainbow
        gradient = range(0, 255, 255//num_tasks)
        task_dict = {}
        for index, taskname in enumerate(task_name_list):
            task_dict[taskname] = gradient[index]

        BARWIDTH = 8
        y_ticks = range(5, 85, 10)

        fig, ax = plt.subplots()
        plt.title("Tile {0}".format(self.tileno))

        # Create a list of broken bars per core per task
        core_list =  [{} for _ in range(8)]

        for index, core in enumerate(self.records_list):
            for taskname in task_name_list:
                core_list[index][taskname] = []
            last_tick = 0
            last_taskname = ""
            for rec in core:
                cur_tick = int(rec["rectime"])

                if rec["switch"] == "IN":
                    last_tick = cur_tick
                    last_taskname = rec["name"]
                elif rec["switch"] == "OUT":
                    core_list[index][rec["name"]].append((last_tick, cur_tick - last_tick))

            # For each task fill in the last duration
            if last_taskname:
                core_list[index][last_taskname].append((last_tick, last_rec_time - last_tick))

            # Add bars
            for taskname in task_name_list:
                ax.broken_barh(core_list[index][taskname], (y_ticks[int(index)]-(BARWIDTH/2), BARWIDTH), facecolors=cmap(task_dict[taskname]))

        ax.set_ylim(0, 80)
        ax.set_xlim(0, last_rec_time)
        ax.set_xlabel('Ticks')
        ax.set_yticks(y_ticks)
        ax.set_yticklabels((range(8)))
        ax.grid(True)

        ax.legend(task_name_list, frameon=True, loc="upper right", title="Task Name")
        ax.invert_yaxis()

        plt.show()

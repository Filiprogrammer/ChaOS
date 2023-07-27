import gdb
import tkinter
from tkinter import *
from tkinter.ttk import *
import _thread
from collections import namedtuple

Task = namedtuple("Task", "id page_directory cpu_time_used last_active threads next_threadId")
Thread = namedtuple("Thread", "id esp ss kernel_stack stack_begin stack_end FPUPtr parent timeout nice")

def on_closing():
    window.quit()

def on_treeview_click(event):
    if event.widget.identify_region(event.x, event.y) == "separator":
        return "break"

def on_label_configure(event):
    event.widget.configure(wraplength=event.width)

def get_tags_for_task(task_id):
    if (task_id == kernel_task_id):
        return ["kernel"]

    if (task_id == do_nothing_task_id):
        return ["do_nothing"]

    return []

def update_gui():
    global shouldUpdate
    window.after(1000, update_gui)

    if (shouldUpdate):
        shouldUpdate = False

        for i in range(queue_number):
            treeThreadQueues[i].delete(*treeThreadQueues[i].get_children())

            for j in range(len(queues[i])):
                thread = queues[i][j]
                tags = get_tags_for_task(thread.parent)
                treeThreadQueues[i].insert("", "end", text=str(thread.id), values=[thread.esp, thread.ss, thread.kernel_stack, thread.stack_begin, thread.stack_end, thread.FPUPtr, "task id: " + str(thread.parent), str(thread.timeout), str(thread.nice)], tags=tags)

            treeThreadQueues[i].tag_configure('kernel', background='orange')
            treeThreadQueues[i].tag_configure('do_nothing', background='turquoise')

            treeThreadQueuesSleeping[i].delete(*treeThreadQueuesSleeping[i].get_children())
            
            for j in range(len(queues_sleeping[i])):
                thread = queues_sleeping[i][j]
                tags = get_tags_for_task(thread.parent)
                treeThreadQueuesSleeping[i].insert("", "end", text=str(thread.id), values=[thread.esp, thread.ss, thread.kernel_stack, thread.stack_begin, thread.stack_end, thread.FPUPtr, "task id: " + str(thread.parent), str(thread.timeout), str(thread.nice)], tags=tags)

            treeThreadQueuesSleeping[i].tag_configure('kernel', background='orange')
            treeThreadQueuesSleeping[i].tag_configure('do_nothing', background='turquoise')

        treeTasks.delete(*treeTasks.get_children())

        for t in tasks:
            tags = get_tags_for_task(t.id)
            treeTasks.insert("", "end", text=str(t.id), values=[t.page_directory, t.cpu_time_used, t.last_active, t.threads, t.next_threadId], tags=tags)

        treeTasks.tag_configure('kernel', background='orange')
        treeTasks.tag_configure('do_nothing', background='turquoise')

        lblCurrentTask.configure(text=("Current thread: " + current_thread_str))
        lblKernelTask.configure(text=("Kernel task: " + kernel_task_str))
        lblDoNothingTask.configure(text=("Do Nothing task: " + do_nothing_task_str))

def setup_treeview_thread_queue(treeview):
    treeview["columns"] = ["one", "two", "three", "four", "five", "six", "seven", "eight", "nine"]
    treeview.column("#0", width=45, minwidth=45, stretch=True)
    treeview.column("one", width=70, minwidth=70, stretch=True)
    treeview.column("two", width=70, minwidth=70, stretch=True)
    treeview.column("three", width=73, minwidth=73, stretch=True)
    treeview.column("four", width=70, minwidth=70, stretch=True)
    treeview.column("five", width=70, minwidth=70, stretch=True)
    treeview.column("six", width=70, minwidth=70, stretch=True)
    treeview.column("seven", width=70, minwidth=70, stretch=True)
    treeview.column("eight", width=60, minwidth=60, stretch=True)
    treeview.column("nine", width=50, minwidth=50, stretch=True)
    treeview.heading("#0", text="id", anchor=tkinter.W)
    treeview.heading("one", text="esp", anchor=tkinter.W)
    treeview.heading("two", text="ss", anchor=tkinter.W)
    treeview.heading("three", text="Kernel Stack", anchor=tkinter.W)
    treeview.heading("four", text="Stack Begin", anchor=tkinter.W)
    treeview.heading("five", text="Stack End", anchor=tkinter.W)
    treeview.heading("six", text="FPUPtr", anchor=tkinter.W)
    treeview.heading("seven", text="Parent Task", anchor=tkinter.W)
    treeview.heading("eight", text="Timeout", anchor=tkinter.W)
    treeview.heading("nine", text="nice", anchor=tkinter.W)
    treeview.bind('<Button-1>', on_treeview_click)

def run_gui():
    global window
    window = tkinter.Tk()
    window.geometry("1300x600")
    window.title("ChaOS Tasks")
    global lblKernelTask
    lblKernelTask = Label(window, text="Kernel task: ")
    lblKernelTask.grid(column=0, row=0, sticky="EW")
    lblKernelTask.bind("<Configure>", on_label_configure)
    global lblDoNothingTask
    lblDoNothingTask = Label(window, text="Do Nothing task: ")
    lblDoNothingTask.grid(column=1, row=0, sticky="EW")
    lblDoNothingTask.bind("<Configure>", on_label_configure)
    global lblCurrentTask
    lblCurrentTask = Label(window, text="Current thread: ")
    lblCurrentTask.grid(column=0, row=1, sticky="EW")
    lblCurrentTask.bind("<Configure>", on_label_configure)
    global lblQueues
    lblQueues = Label(window, text="Thread Queues")
    lblQueues.grid(column=0, row=2, sticky="EW")
    global lblQueuesSleeping
    lblQueuesSleeping = Label(window, text="Thread Queues Sleeping")
    lblQueuesSleeping.grid(column=1, row=2, sticky="EW")

    global treeThreadQueues
    global treeThreadQueuesSleeping
    treeThreadQueues = []
    treeThreadQueuesSleeping = []

    for i in range(queue_number):
        treeThreadQueues.append(Treeview(window, height=20))
        setup_treeview_thread_queue(treeThreadQueues[i])
        treeThreadQueues[i].grid(column=0, row=3+i, sticky="EW")
        treeThreadQueuesSleeping.append(Treeview(window, height=20))
        setup_treeview_thread_queue(treeThreadQueuesSleeping[i])
        treeThreadQueuesSleeping[i].grid(column=1, row=3+i, sticky="EW")

    global treeTasks
    treeTasks = Treeview(window, height=20)
    treeTasks["columns"] = ["one", "two", "three", "four", "five"]
    treeTasks.column("#0", width=45, minwidth=45, stretch=True)
    treeTasks.column("one", width=70, minwidth=70, stretch=True)
    treeTasks.column("two", width=80, minwidth=80, stretch=True)
    treeTasks.column("three", width=80, minwidth=80, stretch=True)
    treeTasks.column("four", width=70, minwidth=70, stretch=True)
    treeTasks.column("five", width=70, minwidth=70, stretch=True)
    treeTasks.heading("#0", text="id", anchor=tkinter.W)
    treeTasks.heading("one", text="page directory", anchor=tkinter.W)
    treeTasks.heading("two", text="CPU time used", anchor=tkinter.W)
    treeTasks.heading("three", text="last active", anchor=tkinter.W)
    treeTasks.heading("four", text="threads", anchor=tkinter.W)
    treeTasks.heading("five", text="next thread id", anchor=tkinter.W)
    treeTasks.bind('<Button-1>', on_treeview_click)
    treeTasks.grid(column=0, row=3+queue_number, sticky="EW")
    window.grid_rowconfigure(3+queue_number, weight=2, uniform="fred")

    window.grid_columnconfigure(0, weight=1, uniform="fred")
    window.grid_columnconfigure(1, weight=1, uniform="fred")

    for i in range(queue_number):
        window.grid_rowconfigure(3+i, weight=1, uniform="fred")

    global shouldUpdate
    shouldUpdate = True
    window.after(1000, update_gui)
    window.protocol("WM_DELETE_WINDOW", on_closing)
    window.mainloop()
    gdb.events.stop.disconnect(stop_handler)
    tkinter._default_root = None
    return

def stop_handler(event):
    read_task_queues()
    global shouldUpdate
    shouldUpdate = True

def add_task(task):
    taskAlreadyExists = False

    for t in tasks:
        if (t.id == task["id"]):
            taskAlreadyExists = True
            break

    if (taskAlreadyExists == False):
        task_id = int(task["id"])
        task_page_directory = hex(int(task["page_directory"])).rstrip("L")
        task_cpu_time_used = str(task["cpu_time_used"]) + u"\u00B5s"
        task_last_active = int(task["last_active"])
        task_threads = hex(int(task["threads"])).rstrip("L")
        task_next_threadId = int(task["next_threadId"])
        tasks.append(Task(id=task_id, page_directory=task_page_directory, cpu_time_used=task_cpu_time_used, last_active=task_last_active, threads=task_threads, next_threadId=task_next_threadId))

def add_thread_to_queue(queue, thread):
    thread_id = int(thread["id"])
    thread_esp = hex(int(thread["esp"])).rstrip("L")
    thread_ss = hex(int(thread["ss"])).rstrip("L")
    thread_kernel_stack = hex(int(thread["kernel_stack"])).rstrip("L")
    thread_stack_begin = hex(int(thread["stack_begin"])).rstrip("L")
    thread_stack_end = hex(int(thread["stack_end"])).rstrip("L")
    thread_FPUPtr = hex(int(thread["FPUPtr"])).rstrip("L")
    thread_parent = int(thread["parent"]["id"])
    thread_timeout = int(thread["timeout"])
    thread_nice = int(thread["nice"])

    queue.append(Thread(id=thread_id, esp=thread_esp, ss=thread_ss, kernel_stack=thread_kernel_stack, stack_begin=thread_stack_begin, stack_end=thread_stack_end, FPUPtr=thread_FPUPtr, parent=thread_parent, timeout=thread_timeout, nice=thread_nice))

def read_task_queues():
    global queue_number
    queue_number = int(gdb.execute("p sizeof(queues) / sizeof(queue_t)", to_string=True).split()[2])

    global queues
    global queues_sleeping
    global tasks
    queues = []
    queues_sleeping = []
    tasks = []

    for i in range(queue_number):
        queues.append([])
        queue = gdb.lookup_symbol("queues")[0].value()[i]
        node = queue["front"]

        while (node != 0):
            thread = node.referenced_value()["data"].cast(gdb.lookup_type("thread_t").pointer()).referenced_value()
            add_thread_to_queue(queues[i], thread)
            add_task(thread["parent"])
            node = node.referenced_value()["next"]

        queues_sleeping.append([])
        queue = gdb.lookup_symbol("queues_sleeping")[0].value()[i]
        node = queue["front"]

        while (node != 0):
            thread = node.referenced_value()["data"].cast(gdb.lookup_type("thread_t").pointer()).referenced_value()
            add_thread_to_queue(queues_sleeping[i], thread)
            add_task(thread["parent"])
            node = node.referenced_value()["next"]

    current_thread = gdb.lookup_symbol("current_thread")[0].value().referenced_value()
    kernel_task = gdb.lookup_symbol("kernel_task")[0].value().referenced_value()
    do_nothing_task = gdb.lookup_symbol("doNothing_task")[0].value().referenced_value()

    add_task(current_thread["parent"])
    add_task(do_nothing_task)

    global kernel_task_id
    global do_nothing_task_id
    kernel_task_id = int(kernel_task["id"])
    do_nothing_task_id = int(do_nothing_task["id"])

    global current_thread_str
    global kernel_task_str
    global do_nothing_task_str
    current_thread_str = current_thread.format_string()
    kernel_task_str = kernel_task.format_string()
    do_nothing_task_str = do_nothing_task.format_string()

read_task_queues()
_thread.start_new_thread(run_gui, ())
gdb.events.stop.connect(stop_handler)

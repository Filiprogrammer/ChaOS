import gdb
import tkinter
from tkinter import *
from tkinter.ttk import *
import _thread
from collections import namedtuple

Region = namedtuple("Region", "size reserved");

def on_closing():
    global window;
    window.quit();

def on_treeview_click(event):
    if event.widget.identify_region(event.x, event.y) == "separator":
        return "break";

def update_gui():
    global shouldUpdate;
    window.after(1000, update_gui);
    if (shouldUpdate):
        shouldUpdate = False;
        lblRegionCount.configure(text="Region Count: " + str(region_count));
        
        treeHeap.delete(*treeHeap.get_children());
        region_base = heap_start;
        
        for i in range(region_count):
            treeHeap.insert("", "end", text="", values=[hex(region_base).rstrip('L'), hex(regions[i].size).rstrip('L') + " (" + str(regions[i].size) + ")", regions[i].reserved]);
            region_base += regions[i].size;

def run_gui():
    global window;
    window = tkinter.Tk();
    window.title("Heap");
    
    global lblRegionCount;
    lblRegionCount = Label(window, text="Region Count: ");
    lblRegionCount.pack();
    
    global treeHeap;
    treeHeap = Treeview(window, height=25);
    treeHeap["columns"] = ["one", "two", "three"];
    treeHeap.column("one", width=100, minwidth=100, stretch=True);
    treeHeap.column("two", width=100, minwidth=100, stretch=True);
    treeHeap.column("three", width=50, minwidth=50, stretch=True);
    treeHeap.heading("one", text="Offset",anchor=tkinter.W);
    treeHeap.heading("two", text="Size",anchor=tkinter.W);
    treeHeap.heading("three", text="Reserved",anchor=tkinter.W);
    treeHeap["show"] = "headings";
    treeHeap.bind('<Button-1>', on_treeview_click);
    treeHeap.pack(side="left", fill="both", expand=True);
    
    treeHeapScroll = Scrollbar(window, orient="vertical", command=treeHeap.yview);
    treeHeapScroll.pack(side="right", fill="y", expand=False);
    treeHeap.configure(yscrollcommand=treeHeapScroll.set);
    
    global shouldUpdate;
    shouldUpdate = True;
    window.after(1000, update_gui);
    window.protocol("WM_DELETE_WINDOW", on_closing);
    window.mainloop();
    gdb.events.stop.disconnect(stop_handler);
    tkinter._default_root = None;
    return;

def stop_handler(event):
    read_heap();
    global shouldUpdate;
    shouldUpdate = True;

def read_heap():
    global heap_start;
    heap_start = int(gdb.lookup_symbol("heap_start")[0].value());
    global region_count;
    region_count = int(gdb.lookup_symbol("region_count")[0].value());
    regions_pointer = gdb.lookup_symbol("regions")[0].value();
    global regions;
    regions = [];
    
    for i in range(region_count):
        region = (regions_pointer + i).referenced_value();
        regions.append(Region(size=int(region["size"]), reserved=str(region["reserved"])));

read_heap();

_thread.start_new_thread(run_gui, ());
gdb.events.stop.connect(stop_handler);

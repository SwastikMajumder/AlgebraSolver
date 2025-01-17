import tkinter as tk
from tkinter import ttk
import math
import numpy as np

from shapely.geometry import LineString, Point, Polygon

def approx_eq(x1, y1, x2, y2):
    if abs(x2-x1)<10 and abs(y2-y1)<10:
        return True
    return False

def between(a, b, c):
    if ((a-1) < b and b < (c+1)) or ((c-1) < b and b < (a+1)):
        return True
    return False

def common_point_handle(x1, y1, x2, y2, x3, y3, x4, y4):
    if approx_eq(x1, y1, x3, y3):
        return between(x2, x1, x4) and between(y2, y1, y4)
    elif approx_eq(x1, y1, x4, y4):
        return between(x2, x1, x3) and between(y2, y1, y3)
    elif approx_eq(x2, y2, x3, y3):
        return between(x1, x2, x4) and between(y1, y2, y4)
    elif approx_eq(x2, y2, x4, y4):
        return between(x1, x2, x3) and between(y1, y2, y3)

def common_point(x1, y1, x2, y2, x3, y3, x4, y4):
    if approx_eq(x1, y1, x3, y3):
        return (x4, y4)
    elif approx_eq(x1, y1, x4, y4):
        return (x3, y3)
    elif approx_eq(x2, y2, x3, y3):
        return (x4, y4)
    elif approx_eq(x2, y2, x4, y4):
        return (x3, y3)
    return None

def find_polygon(lines, curr_point, history, i_hist):
    tmp = None
    for i in range(len(lines)):
        if curr_point in lines[i]:
            for j in range(len(lines[i])):
                if curr_point == lines[i][j]:
                    if j+1 != len(lines[i]):
                        if lines[i][j+1] not in history:
                            tmp = find_polygon(lines, lines[i][j+1], history + [curr_point], i_hist + [i])
                            if tmp is not None:
                                return tmp + [curr_point]
                        elif history[0]==lines[i][j+1] and len(set(i_hist)) > 1:
                            return [curr_point]
                    if j-1 != -1:
                        if lines[i][j-1] not in history:
                            tmp = find_polygon(lines, lines[i][j-1], history + [curr_point], i_hist + [i])
                            if tmp is not None:
                                return tmp + [curr_point]
                        elif history[0]==lines[i][j-1] and len(set(i_hist)) > 1:
                            return [curr_point]
    return None

def is_dup(lines, polygon):
    if len(polygon) == 3:
        return False
    for i in range(len(polygon)-2):
        for j in range(i+2, len(polygon)):
            if i==0 and j==len(polygon)-1:
                continue
            for k in range(len(lines)):
                if (polygon[i] in lines[k]) and (polygon[j] in lines[k]):
                    count = 0
                    for l in range(i, j):
                        if polygon[l] not in lines[k]:
                            count += 1
                            break
                    if j != len(polygon)-1:
                        for l in range(j+1, len(polygon)):
                            if count == 1 and (polygon[l] not in lines[k]):
                                return True
                    if i != 0:
                        for l in range(0, i):
                            if count == 1 and (polygon[l] not in lines[k]):
                                return True
    return False

def find_all(lines, max_point):
    all_polygon = []
    for i in range(max_point):
        tmp = find_polygon(lines, i, [], [])
        if tmp is not None:
            if len(all_polygon) == 0:
                all_polygon.append(tmp)
                continue
            for i in range(len(all_polygon)):
                if set(all_polygon[i])==set(tmp):
                    break
                elif i == len(all_polygon)-1:
                    all_polygon.append(tmp)
                    break
    for i in range(len(all_polygon)-1, -1, -1):
        if is_dup(lines, all_polygon[i]):
            all_polygon.pop(i)
    return all_polygon

def is_circle_line_segment_intersection(circle_center_x, circle_center_y, circle_radius, x1, y1, x2, y2):
    circle_center = Point(circle_center_x, circle_center_y)
    line_segment = LineString([(x1, y1), (x2, y2)])
    distance_to_line = circle_center.distance(line_segment)
    return distance_to_line <= circle_radius

class DrawingApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Physics GUI")
        self.canvas = tk.Canvas(self.root, width=400, height=400, bg="white")
        self.canvas.pack()
        self.option_var = tk.StringVar(value="draw_mode")  # Default option selected
        self.option_var.trace_add('write', self.on_radio_button_selected)
        tk.Radiobutton(self.root, text="Draw Mode", variable=self.option_var, value="draw_mode").pack()
        tk.Radiobutton(self.root, text="Surface Select", variable=self.option_var, value="surface_select").pack()
        tk.Radiobutton(self.root, text="Object Select", variable=self.option_var, value="object_select").pack()
        tk.Radiobutton(self.root, text="Angle Select", variable=self.option_var, value="angle_select").pack()
        self.label_text = tk.StringVar(value="--none selected--")
        self.label = tk.Label(self.root, textvariable=self.label_text)
        self.label.pack()
        self.select_multiple_var = tk.BooleanVar(value=False)
        self.select_multiple_checkbox = tk.Checkbutton(self.root, text="Select Multiple", variable=self.select_multiple_var)
        self.select_multiple_checkbox.pack()
        self.new_window_button = tk.Button(self.root, text="Properties", command=self.open_new_window)
        self.new_window_button.pack()
        self.start_point = None
        self.points = []
        self.lines = []
        self.current_line = None
        self.current_select = None
        self.selection_type = None
        self.selection_string = None
        self.draw_initial_line()
        self.property_data = {}
        self.canvas.bind("<Button-1>", self.on_left_click)
        self.canvas.bind("<B1-Motion>", self.on_left_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_left_release)
        self.canvas.bind("<Button-3>", self.on_right_click)
    def apply_properties(self):
        self.property_data.update({self.selection_string : [self.selected_option.get(), self.entry_var.get(), self.selected_option2.get(), self.entry_var2.get()]})
        print(self.property_data)
        
    def apply_properties_2(self):
        self.property_data.update({self.selection_string : self.checkbox_var.get()})
        print(self.property_data)
        
    def open_new_window(self):
        new_window = tk.Toplevel(self.root)
        selected_mode = self.option_var.get()
        if selected_mode == "surface_select":
            new_window.title("Properties " + "(surface " + self.selection_string + ")")
        elif selected_mode == "object_select":
            new_window.title("Properties " + "(object " + self.selection_string + ")")
        elif selected_mode == "angle_select":
            new_window.title("Properties " + "(angle " + self.selection_string + ")")
        new_window.geometry("300x200")
        if  selected_mode == "object_select":
            options = ["Point Mass", "Rigid Mass"]
            self.selected_option = tk.StringVar()
            dropdown = ttk.Combobox(new_window, values=options, textvariable=self.selected_option, state="readonly")
            dropdown.pack()
            label = tk.Label(new_window, text="Mass")
            label.pack()
            self.entry_var = tk.StringVar()
            entry = tk.Entry(new_window, textvariable=self.entry_var)
            entry.pack()
            label2 = tk.Label(new_window, text="Relative Position")
            label2.pack()
            options2 = ["Ground"]
            self.selected_option2 = tk.StringVar()
            dropdown2 = ttk.Combobox(new_window, values=options2, textvariable=self.selected_option2, state="readonly")
            dropdown2.pack()
            self.entry_var2 = tk.StringVar()
            entry2 = tk.Entry(new_window, textvariable=self.entry_var2)
            entry2.pack()
            button = tk.Button(new_window, text="Apply", command=self.apply_properties)
            button.pack()
            if self.selection_string in self.property_data.keys():
                tmp = self.property_data[self.selection_string]
                self.selected_option.set(tmp[0])
                self.entry_var.set(tmp[1])
                self.selected_option2.set(tmp[2])
                self.entry_var2.set(tmp[3])
        elif selected_mode == "surface_select":
            self.checkbox_var = tk.BooleanVar()
            tk.Checkbutton(new_window, text="Glued", variable=self.checkbox_var).pack()
            tk.Button(new_window, text="Apply", command=self.apply_properties_2).pack()
            if self.selection_string in self.property_data.keys():
                self.checkbox_var.set(self.property_data[self.selection_string])
        
    def on_radio_button_selected(self, *args):
        selected_option = self.option_var.get()
        self.label_text.set("--none selected--")
        self.selection_string = None
        self.new_window_button.configure(state="disabled")
        if selected_option == "draw_mode" or selected_option == "object_select":
            self.select_multiple_checkbox.config(state="disabled")
        elif selected_option == "surface_select" or selected_option == "angle_select":
            self.select_multiple_checkbox.config(state="normal")
        
    def draw_initial_line(self):
        self.new_window_button.configure(state="disabled")
        self.select_multiple_checkbox.config(state="disabled")
        x1, y1, x2, y2 = 50, 300, 350, 300
        self.canvas.create_line(x1, y1, x2, y2, width=2, fill="black")
        self.points.append([x1, y1])
        self.canvas.create_oval(x1 - 2, y1 - 2, x1 + 2, y1 + 2, fill="red")
        self.points.append([x2, y2])
        self.canvas.create_oval(x2 - 2, y2 - 2, x2 + 2, y2 + 2, fill="red")
        self.lines.append([0, 1])
        
    def on_left_click(self, event):
        selected_mode = self.option_var.get()
        if selected_mode == "draw_mode":
            for i in range(len(self.points)):
                x, y = self.points[i]
                if abs(event.x - x) <= 5 and abs(event.y - y) <= 5:
                    self.start_point = i
                    return
            self.start_point = None

    def on_left_drag(self, event):
        selected_mode = self.option_var.get()
        if selected_mode == "draw_mode":
            if self.start_point is not None:
                x, y = event.x, event.y
                if self.current_line is not None:
                    self.canvas.delete(self.current_line)
                self.current_line = self.canvas.create_line(self.points[self.start_point][0], self.points[self.start_point][1], x, y, width=2, fill="black")

    def on_left_release(self, event):
        selected_mode = self.option_var.get()
        if selected_mode == "draw_mode" and self.start_point is not None:
            for i in range(len(self.lines)):
                for j in range(len(self.lines[i])-1):
                    [x1, y1], [x2, y2], [x3, y3], [x4, y4] = self.points[self.start_point], [event.x, event.y], self.points[self.lines[i][j]], self.points[self.lines[i][j+1]]
                    tmp = common_point(x1, y1, x2, y2, x3, y3, x4, y4)
                    if tmp is not None:
                        if abs(x1*(y2-tmp[1])+x2*(tmp[1]-y1)+tmp[0]*(y1-y2)) < 500:
                            if common_point_handle(x1, y1, x2, y2, x3, y3, x4, y4) == False:
                                if self.current_line is not None:
                                    self.canvas.delete(self.current_line)
                                self.current_line = None
                                return
                        continue
                    line1 = LineString([(x1, y1), (x2, y2)])
                    line2 = LineString([(x3, y3), (x4, y4)])
                    if line1.intersects(line2):
                        if self.current_line is not None:
                            self.canvas.delete(self.current_line)
                        self.current_line = None
                        return
            if self.current_line is not None:
                for i in range(len(self.points)):
                    x, y = self.points[i]
                    if abs(event.x - x) < 10 and abs(event.y - y) < 10:
                        self.lines.append([i, self.start_point])
                        self.current_line = None
                        print(self.lines)
                        return
            self.points.append([event.x, event.y])
            self.lines.append([len(self.points)-1, self.start_point])
            self.canvas.create_oval(event.x - 2, event.y - 2, event.x + 2, event.y + 2, fill="red")
            self.current_line = None
            print(self.lines)
        elif selected_mode == "surface_select":
            for i in range(len(self.lines)):
                for j in range(len(self.lines[i])-1):
                    [x1, y1], [x2, y2], [x3, y3] = self.points[self.lines[i][j]], self.points[self.lines[i][j+1]], [event.x, event.y]
                    if is_circle_line_segment_intersection(x3, y3, 2, x1, y1, x2, y2):
                        label_string = chr(ord("A")+self.lines[i][j])+chr(ord("A")+self.lines[i][j+1])
                        if self.selection_string is not None and self.select_multiple_var.get():
                            self.selection_string += " + " + label_string
                        else:
                            self.selection_string = label_string
                        self.label_text.set(self.selection_string)
                        return
        elif selected_mode == "object_select":
            all_polygon = find_all(self.lines, len(self.points))
            for i in range(len(all_polygon)):
                tmp = []
                for j in range(len(all_polygon[i])):
                    tmp.append((self.points[all_polygon[i][j]][0], self.points[all_polygon[i][j]][1]))
                print(tmp)
                if Polygon(LineString(tmp)).contains(Point(event.x, event.y)):
                    label_string = ""
                    for j in range(len(all_polygon[i])):
                        label_string += chr(ord("A")+all_polygon[i][j])
                    self.selection_type = "object"
                    self.selection_string = label_string
                    self.new_window_button.configure(state="normal")
                    self.label_text.set(label_string)
        elif selected_mode == "angle_select":
            label_list = []
            for i in range(len(self.lines)-1):
                for j in range(i+1, len(self.lines)):
                    intersect_point = set(self.lines[i]).intersection(set(self.lines[j]))
                    print(intersect_point)
                    for k in list(intersect_point):
                        print(self.lines)
                        x1, y1, x2, y2, x3, y3, x4, y4 = None, None, None, None, None, None, None, None
                        if k != self.lines[i][0]:
                            [x1, y1] = self.points[self.lines[i][0]]
                        if k != self.lines[i][-1]:
                            [x2, y2] = self.points[self.lines[i][-1]]
                        if k != self.lines[j][0]:
                            [x3, y3] = self.points[self.lines[j][0]]
                        if k != self.lines[j][-1]:
                            [x4, y4] = self.points[self.lines[j][-1]]
                        label_string = None
                        if (x1 is not None) and (x3 is not None) and Polygon(LineString([(self.points[k][0], self.points[k][1]), (x1, y1), (x3, y3)])).contains(Point(event.x, event.y)) and abs(self.points[k][0] - event.x) <= 25 and abs(self.points[k][1] - event.y) <= 25:
                            label_string = chr(ord("A")+self.lines[j][0]) + chr(ord("A")+k) + chr(ord("A")+self.lines[i][0])
                        elif (x1 is not None) and (x4 is not None) and Polygon(LineString([(self.points[k][0], self.points[k][1]), (x1, y1), (x4, y4)])).contains(Point(event.x, event.y)) and abs(self.points[k][0] - event.x) <= 25 and abs(self.points[k][1] - event.y) <= 25:
                            label_string = chr(ord("A")+self.lines[j][-1]) + chr(ord("A")+k) + chr(ord("A")+self.lines[i][0])
                        elif (x2 is not None) and (x3 is not None) and Polygon(LineString([(self.points[k][0], self.points[k][1]), (x2, y2), (x3, y3)])).contains(Point(event.x, event.y)) and abs(self.points[k][0] - event.x) <= 25 and abs(self.points[k][1] - event.y) <= 25:
                            label_string = chr(ord("A")+self.lines[j][0]) + chr(ord("A")+k) + chr(ord("A")+self.lines[i][-1])
                        elif (x2 is not None) and (x4 is not None) and Polygon(LineString([(self.points[k][0], self.points[k][1]), (x2, y2), (x4, y4)])).contains(Point(event.x, event.y)) and abs(self.points[k][0] - event.x) <= 25 and abs(self.points[k][1] - event.y) <= 25:
                            label_string = chr(ord("A")+self.lines[j][-1]) + chr(ord("A")+k) + chr(ord("A")+self.lines[i][-1])
                        if label_string is not None:
                            label_list.append(label_string)
            minimum_val = 999
            final_label = None
            for i in label_list:
                vec_a = np.array(self.points[ord(i[0])-ord("A")])-np.array(self.points[ord(i[1])-ord("A")])
                vec_b = np.array(self.points[ord(i[2])-ord("A")])-np.array(self.points[ord(i[1])-ord("A")])
                tmp = np.arccos(np.dot(vec_a,vec_b)/(np.linalg.norm(vec_a)*np.linalg.norm(vec_b)))
                if minimum_val > tmp:
                    minimum_val = tmp
                    final_label = i
            if final_label is not None:
                if self.selection_string is not None and self.select_multiple_var.get():
                    self.selection_string += " + " + final_label
                else:
                    self.selection_string = final_label
                self.label_text.set(self.selection_string)
                
    def on_right_click(self, event):
        selected_mode = self.option_var.get()
        if selected_mode == "draw_mode":
            for i in range(len(self.points)):
                x, y = self.points[i]
                if abs(event.x - x) <= 5 and abs(event.y - y) <= 5:
                    return
            for i in range(len(self.lines)):
                for j in range(len(self.lines[i])-1):
                    [x1, y1], [x2, y2], [x3, y3] = self.points[self.lines[i][j]], self.points[self.lines[i][j+1]], [event.x, event.y]
                    if is_circle_line_segment_intersection(x3, y3, 2, x1, y1, x2, y2):
                        self.points.append([x3, y3])
                        self.canvas.create_oval(x3 - 2, y3 - 2, x3 + 2, y3 + 2, fill="red")
                        self.lines[i].insert(j+1, len(self.points)-1)
                        print(self.lines)
                        return

if __name__ == "__main__":
    root = tk.Tk()
    app = DrawingApp(root)
    root.mainloop()

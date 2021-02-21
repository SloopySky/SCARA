import math
from Communication import *


class GCode:
    pass


class Axis:
    def __init__(self, min_range=0, max_range=0, reduction=1):
        self.current_position = 0.0
        self.min_range = min_range
        self.max_range = max_range
        self.reduction = reduction

    def is_in_range(self, position):
        return not(position > self.max_range or position < self.min_range)


class Wrist(Axis):
    pass


class Gripper:
    pass


class Scara:
    axes_names = ["VERTICAL", "ARM", "FOREARM", "WRIST"]
    cartesian_names = ["X", "Y", "Z"]
    G90 = True          # is_absolute = True
    G91 = False         # is_absolute = False
    is_absolute = G90

    # PARAMETERS:
    l1 = 93.0
    l2 = 106.5
    min_range = {"VERTICAL": 0,
                 "ARM": -190,
                 "FOREARM": -280,
                 "WRIST": 0}
    max_range = {"VERTICAL": 300,
                 "ARM": 190,
                 "FOREARM": 280,
                 "WRIST": 0}

    def __init__(self):
        self.axes = {}
        for name in self.axes_names:
            self.axes[name] = Axis(min_range=self.min_range[name], max_range=self.max_range[name])
        self.xyz = self.forward_kinematics()

    def home(self):
        pass

    def set_absolute(self, absolute=G90):
        self.is_absolute = absolute

    def move(self, values, is_abs=is_absolute):
        absolute = displacement = values

        # Calculate either target positions (if G90)
        # or displacements (if G91):
        if is_abs:
            for name in self.axes_names:
                displacement[name] = absolute[name] - self.axes[name].current_position
        else:
            for name in self.axes_names:
                absolute[name] = displacement[name] + self.axes[name].current_position
        
        # Check if target positions in range:
        for name in self.axes_names:
            if not(self.axes[name].is_in_range(absolute[name])):
                print("Out of range!")
                return
            
        # Store new target positions and move:
        for names in self.axes_names:
            self.axes[names].current_position = absolute[names]

        # Calculate xyz target position:
        self.xyz = self.forward_kinematics()

    def forward_kinematics(self):
        for name in Scara.axes_names:
            if name == "VERTICAL":
                continue
        xyz = {"X": round(self.l1 * math.cos(math.radians(self.axes["ARM"].current_position))
                          + self.l2 * math.cos(math.radians(self.axes["ARM"].current_position
                                                            + self.axes["FOREARM"].current_position)), 1),
               "Y": round(self.l1 * math.sin(math.radians(self.axes["ARM"].current_position))
                          + self.l2 * math.sin(math.radians(self.axes["ARM"].current_position
                                                            + self.axes["FOREARM"].current_position)), 1),
               "Z": round(self.axes["VERTICAL"].current_position, 1)}
        return xyz

    def inverse_kinematics(self, xyz, is_abs=is_absolute):
        if is_abs is self.G91:
            for name in self.cartesian_names:
                # Operate on absolute values inside this function:
                xyz[name] = xyz[name] + self.xyz[name]

        l1 = self.l1
        l2 = self.l2
        x = xyz["X"]
        y = xyz["Y"]

        if l1+l2 < math.sqrt(x*x + y*y):
            # No solution for such point
            # Force out of range:
            values = {"VERTICAL": self.min_range["VERTICAL"] - 10,
                      "ARM": 0,
                      "FOREARM": 0,
                      "WRIST": 0}
        else:
            cos_th2 = (x*x + y*y - l1*l1 - l2*l2) / (2*l1*l2)
            print(cos_th2)

            sin_th2 = math.sqrt(1 - cos_th2 * cos_th2)
            cos_th1_nominator = y*l2*sin_th2 + x*(l1 + l2*cos_th2)
            sin_th1_nominator = y*(l1 + l2*cos_th2) - x*l2*sin_th2
            # denominator = (l1 + l2*cos_th2)*(l1 + l2*cos_th2) + (l2*sin_th2)*(l2*sin_th2)

            th1 = math.degrees(math.atan2(sin_th1_nominator, cos_th1_nominator))
            th2 = math.degrees(math.acos((xyz["X"]*xyz["X"] - self.l1*self.l1 - self.l2*self.l2) / (2*self.l1*self.l2)))

            values = {"VERTICAL": xyz["Z"],
                      "ARM": round(th1, 1),
                      "FOREARM": round(th2, 1),
                      "WRIST": 0}

        if is_abs is self.G91:
            for name in self.axes_names:
                values[name] = values[name] - self.axes[name].current_position

        return values

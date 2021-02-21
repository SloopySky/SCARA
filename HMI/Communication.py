import serial


class Communication:
    is_connected = False
    serial_port = None
    ENCODING = 'UTF-8'

    def __init__(self):
        self.serial_port = serial.Serial(port='/dev/ttyUSB0',
                                         baudrate=38400,
                                         parity=serial.PARITY_NONE,
                                         stopbits=serial.STOPBITS_ONE,
                                         bytesize=serial.EIGHTBITS,
                                         timeout=0.1)
        self.serial_port.flush()

    def transmit(self, message):
        message = (message + '\r').encode(self.ENCODING)
        self.serial_port.write(message)

    def receive(self):
        message = self.serial_port.readline()
        message = message.decode(self.ENCODING)
        print(message)
        return message


class GCode:
    # List of words allowed:
    words_allowed = ["G",               # G0, G1 - Move
                     "M",               # M - Open or Close the gripper
                     "F",               # F - Change feed
                     "W",               # W - Wait
                     "X", "Y", "Z",     # XYZ - Cartesian coordinates
                     "A1", "A2", "A3"]  # A1, A2, A3 - Angular axes

    def __init__(self, scara=None):
        self.scara = scara

    def translate(self, task):
        # Remove spaces, everything uppercase:
        task = task.replace(" ", "").upper()

        words = {}
        # For each word allowed check if is in task:
        for word in self.words_allowed:
            if word in task:
                # Move index indicator to the last char of word,
                i = task.index(word) + len(word)
                words[word] = ""
                # Store chars as values while next char is not the beginning of the next word:
                while not task[i].isalpha():
                    # Store only numeric or '.' or '-' characters, skip the others:
                    if task[i].isnumeric() or task[i] is '.' or task[i] is '-':
                        words[word] = words[word] + task[i]
                    # Next char:
                    i = i + 1
                    # Prevent overflow:
                    if i >= len(task):
                        break
        print(words)

        # Check if gathered data is correct:
        for argument in words.values():
            # Value should be numeric instead of '.' or '-',
            # otherwise there's an error:
            if not(argument.replace('-', "").replace('.', "").isnumeric()):
                print("Wrong task")
                return

        """
        Order:
        F
        X Y Z (if Z goes up, Z as first, otherwise Z as last)
        A
        M
        W
        """
        if 'F' in words:
            pass

    def task(self, task):
        function_code = task[0]
        method = getattr(self, function_code)
        method()


if __name__ == "__main__":
    gcode = GCode()

    cmd = "g0x5.0y0 z-4.5 a3=255 a1=14 F10 M3 C45 W4"
    gcode.translate(cmd)

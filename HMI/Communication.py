import serial
import serial.tools.list_ports as list_ports


class Communication:
    # Serial defaults:
    BAUDRATE = 38400
    TIMEOUT = 0.1
    ENCODING = 'UTF-8'

    serial_port = serial.Serial(baudrate=BAUDRATE,
                                parity=serial.PARITY_NONE,
                                stopbits=serial.STOPBITS_ONE,
                                bytesize=serial.EIGHTBITS,
                                timeout=TIMEOUT)

    connected = False
    ready = False

    buffer = []

    def serial_open(self, port=None):
        try:
            self.serial_port.port = port
            self.serial_port.open()
            if self.serial_port.isOpen():
                self.connected = True
                self.ready = True
                self.serial_port.flush()
                return True
            else:
                raise
        except:
            self.connected = False
            return False

    def ready_check(self):
        # Try:
        if not self.connected:
            return False

        if not self.ready:
            # If there is data to receive:
            if self.serial_port.in_waiting:
                ack = self.serial_port.read(1).decode(self.ENCODING)
                # If '1' received port is ready
                if ack is '1':
                    self.ready = True
            # self.serial_port.reset_input_buffer()

    def connection_check(self):
        if self.serial_port.isOpen():
            self.connected = True
        else:
            self.connected = False

    def to_buffer(self, packet):
        # If packet is not a list:
        if not isinstance(packet, list):
            packet = [packet]
        self.buffer.append(packet)
        print(self.buffer)

    def is_buffer_empty(self):
        return not len(self.buffer)

    def send(self):
        if not self.connected:
            return
        if not self.ready:
            return

        # Check if there is something to send in the buffer
        if not len(self.buffer):
            # Nothing to send
            return

        packet = self.buffer[0]
        self.buffer.pop(0)
        print("Packet: ")
        print(packet)

        # Send packet:
        for value in packet:
            value = str(value) + '\r'
            self.serial_port.write(value.encode(self.ENCODING))

        # Busy...:
        self.ready = False

    @staticmethod
    def get_devices():
        devices = list_ports.comports()
        for i in range(len(devices)):
            devices[i] = devices[i].device
        return devices

#!/usr/bin/env python3

import argparse
import glob
import queue
import re
import threading
import time
import tkinter as tk
from tkinter import font

try:
    import serial
    from serial import SerialException
    from serial.tools import list_ports
except ImportError:  # pragma: no cover
    serial = None
    SerialException = Exception
    list_ports = None

VALUE_MIN = -10
VALUE_MAX = 10
VALUE_PATTERN = re.compile(r"^[+-]?\d+$")
PORT_HINTS = ("usbmodem", "usbserial", "SLAB_USBtoUART", "wchusbserial")


def extract_value(line: str):
    match = VALUE_PATTERN.fullmatch(line.strip())
    if not match:
        return None
    value = int(match.group(0))
    if VALUE_MIN <= value <= VALUE_MAX:
        return value
    return None


def detect_default_port():
    candidates = []

    if list_ports is not None:
        for port in list_ports.comports():
            device = port.device or ""
            if any(hint in device for hint in PORT_HINTS):
                candidates.append(device)
    else:
        for pattern in ("/dev/cu.usbmodem*", "/dev/cu.usbserial*", "/dev/cu.SLAB_USBtoUART*", "/dev/cu.wchusbserial*"):
            candidates.extend(glob.glob(pattern))

    candidates = sorted(set(candidates))
    if len(candidates) == 1:
        return candidates[0], candidates
    return None, candidates


class ValueMonitorApp:
    def __init__(self, root: tk.Tk, args: argparse.Namespace):
        self.root = root
        self.args = args
        self.message_queue = queue.Queue()
        self.running = True

        self.root.title("ESP32 Value Monitor")
        self.root.geometry("520x320")
        self.root.configure(bg="white")

        value_font = font.Font(family="Helvetica", size=150, weight="bold")
        status_font = font.Font(family="Helvetica", size=16)

        self.value_var = tk.StringVar(value="--")
        self.status_var = tk.StringVar(value="waiting for data")

        self.value_label = tk.Label(
            root,
            textvariable=self.value_var,
            font=value_font,
            bg="white",
            fg="black",
        )
        self.value_label.pack(expand=True)

        self.status_label = tk.Label(
            root,
            textvariable=self.status_var,
            font=status_font,
            bg="white",
            fg="#555555",
        )
        self.status_label.pack(pady=(0, 20))

        self.worker = threading.Thread(target=self.read_loop, daemon=True)
        self.worker.start()

        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self.root.after(100, self.process_messages)

    def read_loop(self):
        if self.args.demo:
            self.message_queue.put(("status", "demo mode"))
            self.demo_loop()
            return

        if serial is None:
            self.message_queue.put(("error", "pyserial is not installed"))
            return

        port = self.args.port
        detected_port = None
        detected_ports = []
        if not port:
            detected_port, detected_ports = detect_default_port()
            port = detected_port

        if not port:
            if detected_ports:
                ports_text = ", ".join(detected_ports)
                self.message_queue.put(
                    ("error", f"multiple candidate ports found, specify --port ({ports_text})")
                )
            else:
                self.message_queue.put(("error", "no ESP32-like serial port found"))
            return

        try:
            with serial.Serial(port, self.args.baudrate, timeout=1) as ser:
                self.message_queue.put(
                    ("status", f"connected: {port} @ {self.args.baudrate}")
                )
                while self.running:
                    raw_line = ser.readline().decode("utf-8", errors="ignore").strip()
                    if not raw_line:
                        continue
                    value = extract_value(raw_line)
                    if value is not None:
                        self.message_queue.put(("value", value))
        except (OSError, SerialException) as exc:
            self.message_queue.put(("error", str(exc)))

    def demo_loop(self):
        value = VALUE_MIN
        direction = 1
        while self.running:
            self.message_queue.put(("value", value))
            time.sleep(0.08)
            value += direction
            if value >= VALUE_MAX:
                value = VALUE_MAX
                direction = -1
            elif value <= VALUE_MIN:
                value = VALUE_MIN
                direction = 1

    def process_messages(self):
        while True:
            try:
                message_type, payload = self.message_queue.get_nowait()
            except queue.Empty:
                break

            if message_type == "value":
                self.value_var.set(str(payload))
            elif message_type == "status":
                self.status_var.set(payload)
            elif message_type == "error":
                self.status_var.set(f"error: {payload}")

        if self.running:
            self.root.after(100, self.process_messages)

    def close(self):
        self.running = False
        self.root.destroy()


def parse_args():
    parser = argparse.ArgumentParser(
        description="Read -10 to 10 integer values from an ESP32 serial output and display them."
    )
    parser.add_argument("--port", default=None, help="Serial port. If omitted, auto-detects an ESP32-like port.")
    parser.add_argument("--baudrate", type=int, default=115200)
    parser.add_argument("--demo", action="store_true", help="Run without serial hardware")
    return parser.parse_args()


def main():
    args = parse_args()
    root = tk.Tk()
    app = ValueMonitorApp(root, args)
    root.mainloop()
    app.running = False


if __name__ == "__main__":
    main()

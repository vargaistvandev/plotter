import socket
import json
import time

HOST = "plotter.local"
PORT = 5000

SECRET = "shared_secret_value"


def send(command: str) -> str:

    full_command = f"{SECRET} {command}\n"

    with socket.create_connection((HOST, PORT), timeout=5) as s:

        s.sendall(full_command.encode("utf-8"))

        chunks = []

        while True:
            data = s.recv(1024)

            if not data:
                break

            chunks.append(data)

        response = b"".join(chunks).decode("utf-8").strip()

        print(">", command)
        print("<", response)

        return response


def get_status():
    """
    Read JSON status from plotter.
    """

    response = send("STATUS")

    try:
        return json.loads(response)
    except json.JSONDecodeError:
        print("Invalid JSON!")
        return None


# ---------------- Demo ----------------

print("Reading initial status...")
status = get_status()

if status:
    print("Motor 1:", status["motor1"])
    print("Motor 2:", status["motor2"])
    print("Battery:", status["batteryPercent"], "%")

print("\nPen down...")
send("PEN DOWN")

time.sleep(1)

print("\nMove to 1000, 1000")
send("MOVE 1000 1000")

time.sleep(1)

print("\nMove to 2000, 500")
send("MOVE 2000 500")

time.sleep(1)

print("\nPen up...")
send("PEN UP")

time.sleep(1)

print("\nStopping...")
send("STOP")

print("\nFinal status:")
status = get_status()

if status:
    print(json.dumps(status, indent=2))
import requests
import time

BASE = "http://plotter.local"   # or use IP if needed

def send(cmd):
    requests.get(f"{BASE}{cmd}", timeout=2)

# ---- Sequence ----
send("/down")          # pen down
time.sleep(0.3)        # let servo settle

send("/move?x=10&y=5") # small move
time.sleep(1)          # wait until movement finishes

send("/up")            # pen up

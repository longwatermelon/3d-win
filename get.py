import pyautogui
import sys
import os
import time

while True:
    x, y = pyautogui.position()

    if (os.path.exists("mcoords_ready")):
        mcoords = open("mcoords", "r").read().split()
        mcoords[0] = int(mcoords[0])
        mcoords[1] = int(mcoords[1])

    pyautogui.moveTo(mcoords[0], mcoords[1])

    if (os.path.exists("click")):
        pyautogui.click()
        os.remove("click")
    # pyautogui.moveTo(100, 100)
    print(mcoords[0], mcoords[1])
    if os.path.exists("ready"):
        os.remove("ready")
    if os.path.exists("out.png"):
        os.remove("out.png")

    pyautogui.screenshot("out.png", region=(0, 0, 940, 593))
    open("ready", "w").close()
    pyautogui.moveTo(x, y)
    # time.sleep(0.4)


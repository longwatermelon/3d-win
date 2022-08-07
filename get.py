import pyautogui
import sys
import os

os.remove("out.png")
x, y = pyautogui.position()
pyautogui.moveTo(int(sys.argv[1]), int(sys.argv[2]))
pyautogui.screenshot("out.png", region=(0, 0, 979, 720))
pyautogui.moveTo(x, y)


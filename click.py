import pyautogui
import sys

x, y = pyautogui.position()
pyautogui.moveTo(int(sys.argv[1]), int(sys.argv[2]))
pyautogui.click()
pyautogui.moveTo(x, y)


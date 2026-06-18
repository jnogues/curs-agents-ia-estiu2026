# This file is executed on every boot (including wake-boot from deepsleep)
#import esp
#esp.osdebug(None)
import os, machine
#os.dupterm(None, 1) # disable REPL on UART(0)
import gc
#import webrepl
#webrepl.start()
gc.collect()

# Manté GPIO0 (flash button) a GND durant el reset -> entra al REPL
# i no executa main.py
if machine.Pin(0, machine.Pin.IN, machine.Pin.PULL_UP).value() == 0:
    import sys
    sys.exit()

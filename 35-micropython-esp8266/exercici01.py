import uasyncio as asyncio
import gc
from machine import Pin

gc.collect()
print("Mem lliure:", gc.mem_free(), "bytes")

led1 = Pin(16, Pin.OUT)
led2 = Pin(0, Pin.OUT)
led3 = Pin(2, Pin.OUT)
led4 = Pin(13, Pin.OUT)

async def tasca_led1():
    while True:
        led1.on()
        await asyncio.sleep_ms(97)
        led1.off()
        await asyncio.sleep_ms(97)

async def tasca_led2():
    while True:
        led2.on()
        await asyncio.sleep_ms(199)
        led2.off()
        await asyncio.sleep_ms(199)

async def tasca_led3():
    while True:
        led3.on()
        await asyncio.sleep_ms(311)
        led3.off()
        await asyncio.sleep_ms(311)

async def tasca_led4():
    while True:
        led4.on()
        await asyncio.sleep_ms(509)
        led4.off()
        await asyncio.sleep_ms(509)

async def monitor_mem():
    while True:
        gc.collect()
        await asyncio.sleep(5)
        print("Mem:", gc.mem_free(), "lliure,", gc.mem_alloc(), "usada")

async def main():
    t1 = asyncio.create_task(tasca_led1())
    t2 = asyncio.create_task(tasca_led2())
    t3 = asyncio.create_task(tasca_led3())
    t4 = asyncio.create_task(tasca_led4())
    tm = asyncio.create_task(monitor_mem())
    await asyncio.gather(t1, t2, t3, t4, tm)

try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Aturat")

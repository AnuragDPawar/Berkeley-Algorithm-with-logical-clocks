import os
import time

for i in range(1,6):
    print("spawning client {}\n".format(i))
    os.system("./client &")
    time.sleep(0.3)
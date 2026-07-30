import subprocess
import sys
import os
import shutil

def run(cmd):
    print(f"[BUILD] {cmd}")
    subprocess.check_call(cmd, shell=True)

# Добавляем snap cmake в PATH
os.environ["PATH"] = "/snap/bin:" + os.environ["PATH"]

# Удаляем старый build
if os.path.exists("build"):
    print("[BUILD] Removing old build directory...")
    shutil.rmtree("build")

# Конфигурируем проект
run("cmake -S . -B build")

# Собираем движок
run("cmake --build build --target aniva_crash --parallel")

# Собираем bootstrap
run("cmake --build build --target bootstrap --parallel")

print("[BUILD] Done.")

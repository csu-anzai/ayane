import os
import re
import shutil
import subprocess


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


# version

for s in read_lines("main.cc"):
    m = re.match('#define version "(.+)"', s)
    if m:
        version = m[1]
if not version:
    print("main.cc: version not defined")
    exit(1)

# configure.ac

xs = read_lines("configure.ac")
xs[0] = (
    "AC_INIT([Ayane], [%s], [russell.wallace@gmail.com], [ayane], [https://github.com/russellw/ayane])"
    % version
)
write_lines("configure.ac", xs)

# Makefile.am

xs = read_lines("Makefile.am")

i = 0
while not xs[i].startswith("ayane_SOURCES "):
    i += 1
i += 1

j = i
while xs[j].startswith("\t"):
    j += 1
del xs[i:j]

ys = []
for root, dirs, files in os.walk("."):
    for filename in files:
        if os.path.splitext(filename)[1] in (".cc", ".h"):
            ys.append("\t" + filename)
for j in range(len(ys) - 1):
    ys[j] += "\\"
xs[i:i] = ys

write_lines("Makefile.am", xs)

# build

subprocess.check_call("release.bat")

# zip

d = "ayane-" + version
if os.path.exists(d):
    shutil.rmtree(d)
os.mkdir(d)

subprocess.check_call("copy *.exe " + d, shell=1)
subprocess.check_call("copy *.md " + d, shell=1)
subprocess.check_call("copy LICENSE " + d, shell=1)

subprocess.check_call("del *.zip", shell=1)
subprocess.check_call("7z a " + d + ".zip " + d)

shutil.rmtree(d)

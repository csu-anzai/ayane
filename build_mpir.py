# change this to specify a different Visual Studio version
mpirdir = r"\mpir\msvc\vs17"

# change this to specify a particular SDK
sdk = ""

# or look for installed SDKs
sdkdir = r"\Windows Kits\10\Include"

import os
import re
import subprocess


def checkdir(d):
    if not os.path.isdir(d):
        print(d + ": not an existing folder")
        exit(1)


# check SDK


def sdkstr(s):
    return ".".join(str(x) for x in s)


if not sdk:
    checkdir(sdkdir)
    sdks = []
    for root, dirs, files in os.walk(sdkdir):
        for d in dirs:
            m = re.match(r"(\d+)\.(\d+)\.(\d+)\.(\d+)", d)
            if m:
                s = []
                s.append(int(m[1]))
                s.append(int(m[2]))
                s.append(int(m[3]))
                s.append(int(m[4]))
                sdks.append(s)
                print("Found " + os.path.join(root, d))
    if not sdks:
        print(sdkdir + ": found no SDKs")
        exit(1)
    sdks.sort()
    sdk = sdkstr(sdks[-1])

print("SDK=" + sdk)

# check MPIR

checkdir(mpirdir)

# specify SDK


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


nchanged = 0
for root, dirs, files in os.walk(mpirdir):
    for f in files:
        if os.path.splitext(f)[1] == ".vcxproj":
            f = os.path.join(root, f)
            lines = read_lines(f)
            changed = False
            for i in range(len(lines)):
                s = lines[i]
                m = re.match(
                    r"(\s*)<WindowsTargetPlatformVersion>(\d+\.\d+\.\d+\.\d+)</WindowsTargetPlatformVersion>",
                    s,
                )
                if m and m[2] != sdk:
                    s = (
                        m[1]
                        + "<WindowsTargetPlatformVersion>"
                        + sdk
                        + "</WindowsTargetPlatformVersion>"
                    )
                    changed = True
                lines[i] = s
            if changed:
                write_lines(f, lines)
                nchanged += 1
print(mpirdir + ": changed " + str(nchanged) + " .vcxproj files")

# build


def build(flags, f):
    c = "MSBuild.exe " + flags + " " + mpirdir + "\\" + f
    print()
    print(c)
    subprocess.check_call(c)


# build generic C version
# select a more specific file for faster architecture-specific version
build("/p:Platform=Win32 /p:Configuration=Debug", r"lib_mpir_gc\lib_mpir_gc.vcxproj")
build("/p:Platform=Win32 /p:Configuration=Release", r"lib_mpir_gc\lib_mpir_gc.vcxproj")
build("/p:Platform=x64 /p:Configuration=Debug", r"lib_mpir_gc\lib_mpir_gc.vcxproj")
build("/p:Platform=x64 /p:Configuration=Release", r"lib_mpir_gc\lib_mpir_gc.vcxproj")

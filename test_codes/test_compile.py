import subprocess
import re
import sys
import os
from glob import glob

def to_hex(data, zero = True):
	tmp = hex(data)[2:].upper()
	if len(tmp) % 2 != 0: tmp = "0" + tmp
	if zero: tmp = "0x" + tmp
	return tmp

while True:
		files = glob(input("file path (dont need whole name): ")+'*.s')
		if len(files) == 1:
				file = files[0]
				break
		else:
				files = '\n\t- '.join(files)
				print(f'found files: \n\t- {files}')
				print(f"please choose!")

file_no_ext = os.path.splitext(file)[0]
out_file = os.path.join("out", f"{file_no_ext}.bin")

vasm = [os.path.join("vasm", "linux_vasm6502_oldstyle"), "-Fbin", "-wdc02", "-dotdir", "-esc", "-o", out_file, file]
if os.name == "nt":
	vasm[0] = os.path.join("vasm", "win_vasm6502_oldstyle.exe")

if not os.path.exists("out"): os.mkdir("out")

p = subprocess.Popen(vasm, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
result = p.communicate()[0].decode()
error = p.communicate()[1].decode()

if error != "":
	print(error, file=sys.stderr)
	input()
	exit()

start_address = 0xFFFF - int(re.search(r':\s+(\d+)', result).group(1)) - 6
start = to_hex(start_address, False)

old_start = ""
pos = -1
with open(file, "r+") as f:
	while True:
		line = f.readline()
		if line == "":
			print("EOF")
			break
		tmp = re.search(r'\s*\.org \$(\S+)', line)
		if tmp != None:
			pos = f.tell() - (len(line) - line.rfind("$"))
			f.seek(pos)
			old_start = tmp.group(1)
			f.write(start)
			break

if pos == -1:
	print("something bad happened", file=sys.stderr)
	input()
	exit()

p = subprocess.Popen(vasm, stdout=subprocess.PIPE)
print(p.communicate()[0].decode())

with open(file, "r+") as f:
	f.seek(pos)
	f.write(old_start)

print(f"final length (ROM start): 0x{start}")

data = []

with open(out_file, "rb") as f:
	while True:
		a = f.read(1)
		if not a:
			break
		data.append(to_hex(a[0]))

vectors = data[-6:]
data = data[:-7]

print("vectors: ")
for i in range(len(vectors) - 1):
	print(f"{vectors[i]}, ", end="")
print(vectors[-1:][0])

print("data: ")
i = 0
for a in range(len(data)):
	i += 1
	if i % 12 != 0: print(f"{data[a]}, ", end="")
	else: print(f"{data[a]},")

input("\npress enter to exit")
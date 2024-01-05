import os

bin_file = "data.bin"
ino_file = f"{os.path.basename(os.path.dirname(os.path.realpath(__file__)))}.ino"

data = ""

with open(bin_file, "rb") as f:
	while True:
		a = f.read(1)
		if not a:
			data = data[:-2]
			break
		a = hex(a[0]).upper()
		if len(a) == 3: a = a[:2] + "0" + a[2]
		data += a + ", "

before_del = []
found = False
after_del = []

with open(ino_file, "r+", encoding="utf-8") as f:
	while True:
		line = f.readline()
		if line == "":
			f.seek(0)
			f.truncate()
			f.writelines(before_del)
			comment = before_del[len(before_del)-1]
			ident = ""
			for i in range(len(comment)):
				if(not comment[i].isspace()):
					break
				ident += comment[i]
			f.write(f"{ident}const PROGMEM uint8_t ROM_data[ROM_LENGTH] = {{ {data} }};\n")
			f.writelines(after_del)
			break
		if found: 
			after_del.append(line)
			continue
		before_del.append(line)
		slash = line.find("//")
		if slash != -1 and line.find("bin file", slash) != -1:
			under_line = f.readline()
			if under_line != "":
				text = under_line.find("const PROGMEM uint8_t ROM_data")
				if text == -1:
					after_del.append(under_line)
				found = True
#!/usr/bin/env python3
#
# setup.py
#
# Creates a local copy of the btstack source code in the src directory which 
# follows the Arduino conventions
#
import os, command, shutil
from subprocess import run

# get latest original source code
def execute_git(fromUrl, name):
    if not os.path.exists(name):
        res = command.run(["git", "clone", fromUrl, name]) 
    else:
        currentDir = os.getcwd()
        os.chdir(name)
        res = command.run(["git","pull"]) 
        os.chdir(currentDir)
    print(res.output) 
    return res

# make sure that we have an empty src directory
def clean_src():
    if os.path.exists("src/btstack"):
        shutil.rmtree("src/btstack")
    if not os.path.exists("src/btstack"):
        os.mkdir("src")
        os.mkdir("src/btstack")

# copy all relevant files
def copy_files():
    shutil.copytree('original/btstack/src', 'src/btstack', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/bluedroid/decoder/include', 'src/btstack/3rd-party/codec', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/bluedroid/decoder/srce', 'src/btstack/3rd-party/codec', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/bluedroid/encoder/include', 'src/btstack/3rd-party/codec', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/bluedroid/encoder/srce', 'src/btstack/3rd-party/codec', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/rijndael', 'src/btstack/3rd-party/rinjndael', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/micro-ecc', 'src/btstack/3rd-party/micro-eec', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/md5', 'src/btstack/3rd-party/md5', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/3rd-party/yxml', 'src/btstack/3rd-party/yxml', dirs_exist_ok=True) 


# deletes a file if it exists
def remove(file):
    if os.path.exists(file):
        os.remove(file)

def remove_dir(dir):
    shutil.rmtree(dir)

# checks if the file is valid
def is_vaid_file(name):
    return name.endswith(".h") or name.endswith(".c") or name.endswith(".inc")

# delete unnecessary files and directories
def cleanup(root):
    # remove("src/libespeak-ng/mbrowrap.c")
    for path, subdirs, files in os.walk(root):
        for name in files:
            filewithpath = os.path.join(path, name)
            if not is_vaid_file(name):
                print("remove:",filewithpath)
                remove(filewithpath)

# wrap the file with a ifdef
def apply_if(filename, cond):
    text_file = open(filename, "r")
    # read whole file to a string
    text = text_file.read()
    first = "#if "+cond+"\n"
    last = "#endif\n"
    new_text = first + text + last
    text_file.close()
    text_file = open(filename, "w")
    text_file.write(new_text)
    text_file.close()

# find the indicated file by name
def find_file(root, filename):
    for path, subdirs, files in os.walk(root):
        for name in files:
            if filename.endswith(name):
                filewithpath = os.path.join(path, name)
                return filewithpath
    print("file",filename,"not found!" )
    return filename

def fix_includes(root):
    # remove("src/libespeak-ng/mbrowrap.c")
    for path, subdirs, files in os.walk(root):
        for name in files:
            filewithpath = os.path.join(path, name)
            fix_includes_in_file(root, filewithpath)


def fix_line(line, startChar, endChar, new_txt, root):
    start = line.index(startChar)+1
    end = line.rindex(endChar)
    include_name = line[start:end]
    new_name = find_file(root, include_name)
    if new_name.startswith("src/"):
        new_name = new_name[4:]
    print("   ", include_name, "->", new_name)

    newline = "#include \""+new_name+"\""
    new_txt = new_txt.replace(line, newline)
    return new_txt


def fix_includes_in_file(root, fileName):
    text_file = open(fileName, "r")
    # read whole file to a string
    data = text_file.read()
    text_file.close()
    # write updated file
    print("processing", fileName)
    new_txt = data

    for line in data.splitlines():
        if "#include" in line:
            if "\"" in line:
                new_txt = fix_line(line,"\"","\"", new_txt, root)
            if "<" in line:
                new_txt = fix_line(line,"<",">", new_txt, root)

    text_file = open(fileName, "w")
    text_file.write(new_txt)
    text_file.close()


##-----------------------
## Main logic starts here
res = execute_git("https://github.com/bluekitchen/btstack", "original/btstack")
if res.exit==0:
    # clean_src()
    copy_files()
    cleanup("src/btstack")
    fix_includes("src/btstack")
    # fix compile errors
    remove_dir("src/btstack/3rd-party/micro-eec/test")
    remove_dir("src/btstack/mesh")
    remove_dir("src/btstack/mesh")
    remove_dir("src/le-audio")
    print("setup completed")
else:
    print("Could not execute git command")

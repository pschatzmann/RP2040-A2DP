#!/usr/bin/env python3
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
    shutil.copytree('original/btstack/src/ble', 'src/btstack/ble', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/src/classic', 'src/btstack/classic', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/src/le-audio', 'src/btstack/le-audio', dirs_exist_ok=True) 
    shutil.copytree('original/btstack/src/mesh', 'src/btstack/mesh', dirs_exist_ok=True) 

# deletes a file if it exists
def remove(file):
    if os.path.exists(file):
        os.remove(file)

# checks if the file is valid
def is_vaid_file(name):
    return name.endswith(".h") or name.endswith(".c")

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
def apply_ifdef(filename, define):
    text_file = open(filename, "r")
    # read whole file to a string
    text = text_file.read()
    text.insert(0,"#if defined("+define+")/n")
    text.add("#endif/n")
    text_file.close()
    text_file = open(filename, "w")
    text_file.write(text)
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
                start = line.index("\"")+1
                end = line.rindex("\"")
                include_name = line[start:end]
                new_name = find_file(root, include_name)
                if new_name.startswith("src/"):
                    new_name = new_name[4:]
                print("   ", include_name, "->", new_name)

                newline = "#include \""+new_name+"\"\n"
                new_txt = new_txt.replace(line, newline)


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
    # create_data()
    print("setup completed")
else:
    print("Could not execute git command")

import os

def fix_file(path):
    with open(path, "rb") as f:
        data = f.read()

    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]

    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        return

    text = text.rstrip("\n") + "\n"

    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)

def walk(root):
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            if name.endswith((".cpp", ".hpp", ".h", ".c", ".cc")):
                fix_file(os.path.join(dirpath, name))

if __name__ == "__main__":
    walk(".")

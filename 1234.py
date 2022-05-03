from contextlib import contextmanager
class File(object):
    def __init__(self, file_name, method):
        self.file_obj = open(file_name, method)
    def __enter__(self):
        return self.file_obj
    def __exit__(self, type, value, traceback):
        self.file_obj.close()
with File("abc21.txt", "w") as opened_file:
    opened_file.write("Hola")

@contextmanager
def open_file(filename, mode):
    f = open(filename, mode)
    yield f
    f.close()

with open_file('abc3.txt', 'w') as f:
    f.write('our second context manager')
    
    

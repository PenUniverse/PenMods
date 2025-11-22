import sys
import os

if len(sys.argv) <= 2:
    print('usage: generate.py <path> <output>')
    exit()

path = sys.argv[1]
output = sys.argv[2]

prefix_count = 0
file_count = 0

result = '<RCC>\n'
result = result + '\t<qresource prefix="/">\n'

def std_dir(url:str):
    url = url.replace('\\','/')
    if url[:1] == '/':
        url = url[1:]
    if url[:1] == '/':
        return std_dir(url)
    return url

for now, dir_list, file_list in os.walk(path):
    dir_list[:] = [d for d in dir_list if d != '.git']

    now = now[len(path):]
    now = std_dir(now)
    
    for fn in file_list:
        fn = std_dir(fn)
        if fn.endswith('.qrc') or fn.endswith('.h'): continue
        p = now
        if p != '':
            p = p + '/'
        result = result + f'\t\t<file>{p}{fn}</file>\n'
        file_count += 1
    prefix_count += 1

result = result + '\t</qresource>\n'

result = result + '</RCC>'

with open(output,'w') as f:
    f.write(result)
    print(f'generated, {prefix_count} prefixes and {file_count} files processed.')

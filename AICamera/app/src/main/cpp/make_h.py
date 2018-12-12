# delete files other than header files
# python3  

import os
import sys

def main(urls):
    for url in urls:
        for root, dirs, files in os.walk(url):
            for name in files:
                suffix = os.path.splitext(name)[1]
                if not suffix == '.h':
                    filename = os.path.join(root, name)
                    os.remove(filename)

if __name__ == '__main__':
    urls = sys.argv[1:]
    main(urls)



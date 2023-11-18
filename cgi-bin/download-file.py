#!/usr/bin/env python3

import cgi
import os
import errno 

# from signal import signal, SIGPIPE, SIG_DFL   
# signal(SIGPIPE,SIG_DFL) 

class Response:
	def __init__(self, content, filename):
		self._res = f"Connection: keep-alive\r\nContent-Length: {len(content)}\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment; filename={filename}\r\n\r\n"

	def __str__(self):
		print(self._res)

def main():

    # Get filename from the query parameter
    filename = os.environ.get('filename')
    fullpath = os.environ.get('fullpath')
    fd = os.environ.get('fd')
    abspath = os.path.abspath("./") + fullpath
   

    if os.path.exists(abspath):
        with open(abspath, 'rb') as f:
            ret = f.read()
           
        res = Response(ret, filename)
        os.write(int(fd), res._res.encode())
        os.write(int(fd), ret)
        f.close()

    else:
        print("Content-Type: text/plain\r")
        print("Status: 404\r")
        print("\r")
        print("Error 404: File not found\r")

if __name__ == '__main__':
    main()
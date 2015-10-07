## Python Web Server for processing messages passed through the OVCLI Proxy
##
## EXAMPLE:

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from SocketServer import ThreadingMixIn
import threading
import argparse
#Regex
import re
import cgi
# Counter for records
counter = 1
 
class LocalData(object):
 records = {}
 
class HTTPRequestHandler(BaseHTTPRequestHandler):
 
 def do_POST(self):
  if re.search('/api/v1/add/', self.path) is not None:
   ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
   if ctype == 'application/json':
    length = int(self.headers.getheader('content-length'))
    data = cgi.parse_qs(self.rfile.read(length), keep_blank_values=1)
    #Allow use of Global Counter
    global counter
    LocalData.records[str(counter)] = data
    print "record %s is added successfully" % counter
    self.send_response(200)
    self.end_headers()
    #After replying to web query, increment counter
    counter = counter + 1
   else:
    data = {}
    self.send_response(200)
    self.end_headers()
  else:
   self.send_response(403)
   self.send_header('Content-Type', 'application/json')
   self.end_headers()
 
  return
 
 def do_GET(self):
  if None != re.search('/api/v1/get/*', self.path):
   recordID = self.path.split('/')[-1]
   if LocalData.records.has_key(recordID):
    self.send_response(200)
    self.send_header('Content-Type', 'application/json')
    self.end_headers()
    self.wfile.write(LocalData.records[recordID])
   else:
    self.send_response(400, 'Bad Request: record does not exist')
    self.send_header('Content-Type', 'application/json')
    self.end_headers()
  elif re.search('/api/v1/recordcount/*', self.path) is not None:
    global counter
    #Access the global counter and print out the amount
    print "Record Count = %s" % counter
  else:
   self.send_response(403)
   self.send_header('Content-Type', 'application/json')
   self.end_headers()
 
  return
 
class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
 allow_reuse_address = True
 
 def shutdown(self):
  self.socket.close()
  HTTPServer.shutdown(self)
 
class SimpleHttpServer():
 def __init__(self, ip, port):
  self.server = ThreadedHTTPServer((ip,port), HTTPRequestHandler)
 
 def start(self):
  self.server_thread = threading.Thread(target=self.server.serve_forever)
  self.server_thread.daemon = True
  self.server_thread.start()
 
 def waitForThread(self):
  self.server_thread.join()
 
 def addRecord(self, recordID, jsonEncodedRecord):
  LocalData.records[recordID] = jsonEncodedRecord
 
 def stop(self):
  self.server.shutdown()
  self.waitForThread()
 
if __name__=='__main__':
 parser = argparse.ArgumentParser(description='HTTP Server')
 parser.add_argument('port', type=int, help='Listening port for HTTP Server')
 parser.add_argument('ip', help='HTTP Server IP')
 args = parser.parse_args()
 
 server = SimpleHttpServer(args.ip, args.port)
 print 'HTTP Server Running...........'
 server.start()
 server.waitForThread()

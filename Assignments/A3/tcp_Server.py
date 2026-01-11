  #!/usr/bin/env python3
  import socket
  import sys

  def start_server(port):
      s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
      s.bind(('0.0.0.0', port))
      s.listen(1)
      print(f"TCP Server listening on port {port}")
      
      while True:
          conn, addr = s.accept()
          print(f"Connection from {addr}")
          data = conn.recv(1024)
          print("Received:", data.decode('utf-8', errors='ignore'))
          conn.close()

  if __name__ == "__main__":
      port = int(sys.argv[1]) if len(sys.argv) > 1 else 5555
      start_server(port)

import socket
import struct

# Define the IP address and port to listen on
IP_ADDRESS = '0.0.0.0'
PORT = 102

# Define the response to send when a request is received
RESPONSE_DATA = b'\x03\x00\x00\x1e\x02\xf0\x80\x32\x01\x00\x00\x01\x00\xc0\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00'

def main():
    # Create a socket to listen for incoming requests
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((IP_ADDRESS, PORT))
    server_socket.listen(1)

    print(f'Listening on {IP_ADDRESS}:{PORT}...')

    while True:
        # Accept incoming connection
        client_socket, client_address = server_socket.accept()
        print(f'Incoming connection from: {client_address[0]}')

        try:
            # Receive the request from the client
            request = client_socket.recv(1024)

            # Print the received request (optional)
            print('Received request:', request.hex())

            # Send the response back to the client
            client_socket.sendall(RESPONSE_DATA)
        except Exception as e:
            print(f'Error occurred: {str(e)}')
        finally:
            # Close the client socket
            client_socket.close()

if __name__ == '__main__':
    main()

import argparse
import logging
import socket
import threading
import datetime

"""Simple Trivia chat room."""
import time
import yaml
import random

LOG = logging.getLogger(__name__)

def client_loop(client, address):
	# message = data.decode("utf-8", errors="ignore").strip()
	while True:
		data = client.recv(1024)
		if not data:
			LOG.warning("Connection to %s lost...", address)
			break

		# The information received (hex bytes not UTF-8) must be converted
		# message_hex = " ".join([hex(byte)[2:].zfill(2) for byte in data])
		message_hex = [hex(byte)[2:].zfill(2) for byte in data]  # type list not str
		print()
		LOG.info("COMMAND received: {} from {}".format(message_hex, address))

		# From a full command match only the command part:
		# Eample: '55 aa 01 00 00 00 00 00   [01 00]   01 01'

		match message_hex[8:10]:
			# Case: Open COMMAND
			case ['01', '00']:
				LOG.info("CASE: ['01', '00']")
				response = bytes.fromhex("55aa01000510000030003001")
				client.send(response)
				LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))

			# Case: Start Enrollment COMMAND
			case ['22', '00']:
				LOG.info("CASE: ['22', '00']")
				LOG.info("5THE ELEMENT: {}".format(int(message_hex[4], 16)))

				# Test Case "fingerprint module DataBase is full":
				# If the (clock) minute is odd -> send to the microcontroller (client) a response with the corresponding
				# error message -> NACK_DB_IS_FULL: 0x10 09
				now = datetime.datetime.now()
				minute = now. minute
				if minute % 2 != 0:
					response = bytes.fromhex("55aa01000910000031004a01")
					client.send(response)
					LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))
				else:
					# Testcase: For this command the Parameter is the ID of the FingerPrint
					#           If the ID is outside the [DEC][0 - 19] -> [0 13][HEX] range, return error NACK_INVALID_POS
					#           If the ID is "1 to 7", return error NACK_IS_ALREADY_USED
					if int(message_hex[4], 16) not in range(0, 19):
						response = bytes.fromhex("55aa01000310000031004401")
						client.send(response)
						LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))
					elif 0 < int(message_hex[4], 16) < 7:
						LOG.info("THIS CASE")
						response = bytes.fromhex("55aa01000510000031004601")
						client.send(response)
						LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))
					else:
						# Everything is OK
						response = bytes.fromhex("55aa01000000000030003001")
						client.send(response)
						LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))
			case _:
				LOG.info("COMMAND not recognized")


def serve_forever(host: str, port: int) -> None:
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	server.setblocking(True)
	server.bind((host, port))
	server.listen(10)

	while True:
		client, address = server.accept()
		LOG.info("A new connection was established. [%s]", address)
		thread = threading.Thread(target=client_loop, args=(client, address))
		thread.daemon = False
		thread.start()


def main() -> None:
	logging.basicConfig(level=logging.INFO)

	parser = argparse.ArgumentParser()
	parser.add_argument("--host", type=str, default="127.0.0.1")
	parser.add_argument("--port", type=int, default=6666)

	args = parser.parse_args()
	try:
		serve_forever(host=args.host, port=args.port)
	except KeyboardInterrupt:
		LOG.warning("The server is shuting down...")


if __name__ == "__main__":
	main()

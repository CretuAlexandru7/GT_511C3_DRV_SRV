import argparse
import logging
import socket
import threading
import datetime

"""Simple Trivia chat room."""
import time
import yaml
import random
# TOPICS: dict[str, list[socket.socket]] = {}
# CLIENTS: dict[str, socket.socket] = {}
# ANSWERS: dict[str, str] = {}
#
#
# def broadcast(message: str, topic_name: str | None = None) -> None:
# 	for topic, client_list in TOPICS.items():
# 		if topic_name and topic != topic_name:
# 			continue
# 		for client in client_list:
# 			client.send(message.encode("utf-8"))


# def join_topic(username: str, topic_name: str):
# 	for client_list in TOPICS.values():
# 		if CLIENTS[username] in client_list:
# 			client_list.remove(CLIENTS[username])
#
# 	new_topic = TOPICS.setdefault(topic_name, [])
# 	new_topic.append(CLIENTS[username])
# 	broadcast(f"> {username} joined the {topic_name} topic.\n", topic_name=topic_name)


# def get_topics(active: bool = False) -> list[str]:
# 	topics: list[str] = []
# 	for topic, clients in TOPICS.items():
# 		if active and not clients:
# 			continue
# 		topics.append(topic)
# 	return topics


# def trivia_loop():
# 	with open("D:\\Desktop\\data.yaml", mode="rb") as file_handle:
# 		data = yaml.safe_load(file_handle)
#
# 	topics = data["topic"]
# 	for topic in topics.keys():
# 		TOPICS.setdefault(topic, [])
#
# 	while True:
# 		for topic in TOPICS:
# 			if topic in ANSWERS:
# 				broadcast(
# 					f"TRIVIA > The answer is: {ANSWERS[topic]}\n", topic_name=topic
# 				)
# 				broadcast(
# 					"TRIVIA > Prepare for the next question...\n", topic_name=topic
# 				)
#
# 			container = topics[topic] if topic in topics else topics["general"]
# 			question = random.choice(container)
# 			ANSWERS[topic] = question["answer"]
# 			broadcast(f"TRIVIA > {question['question']}\n", topic_name=topic)
# 		time.sleep(3000)


# def client_loop(client: socket.socket, address: str):
# 	username: str = ""
# 	topic_name: str = "general"
#
# 	while True:
# 		data = client.recv(1024)
# 		if not data:
# 			LOG.warning("Connection to %s lost...", address)
# 			break
#
# 		message = data.decode("utf-8", errors="ignore").strip()
# 		LOG.info("Retrived %d bytes from %s.", len(message), address)
# 		if not message.startswith("/"):
# 			broadcast(f"{username} > {message}\n", topic_name=topic_name)
# 			continue
#
# 		match message.split():
# 			case ["/name", username]:
# 				CLIENTS[username] = client
# 			case ["/help"]:
# 				message = (
# 					"~ > Available commands:\n"
# 					"\t/name [name]- Update the username.\n"
# 					"\t/help - Get the current message.\n"
# 					"\t/topic [topic-name] - Select the topic you are insterest in.\n"
# 					"\t/show topics - Show all the active topics.\n"
# 					"\t/show users - Show all the active users in the current topic.\n"
# 					"\t/answer [answer] - Answer the current topic question.\n"
# 					"\n"
# 				)
# 				client.send(message.encode("utf-8"))
# 			case ["/topic", topic_name]:
# 				join_topic(username, topic_name=topic_name)
# 			case ["/show", "topics"]:
# 				topics = "\n * ".join(get_topics())
# 				message = f"~ > Active topics for the moment:\n * {topics}\n"
# 				client.send(message.encode("utf-8"))
# 			case ["/answer"]:
# 				pass
# 			case _:
# 				client.send(f"~ > Invalid command: '{message}'\n".encode("utf-8"))
#
# def serve_forever(host: str, port: int) -> None:
# 	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 	server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# 	server.setblocking(True)
# 	server.bind((host, port))
# 	server.listen(10)
#
# 	trivia = threading.Thread(target=trivia_loop)
# 	trivia.daemon = False
# 	trivia.start()
#
# 	while True:
# 		client, address = server.accept()
# 		LOG.info("A new connection was established. [%s]", address)
# 		thread = threading.Thread(target=client_loop, args=(client, address))
# 		thread.daemon = False
# 		thread.start()

#######################################################################################################
#######################################################################################################

# def client_loop(client, address):
#
# 	while True:
# 		data = client.recv(1024)
# 		if not data:
# 			LOG.warning("Connection to %s lost...", address)
# 			break
# 		print("Message from C client: {}".format(data))
# 		message = data.decode("utf-8", errors="ignore").strip()
# 		LOG.info("Retrived %d bytes from %s.", len(message), address)
#
# 		match message.split():
# 			case ["/name"]:
# 				pass
# 			case ["/show", "topics"]:
# 				message = f"~ > Active topics for the moment:\n"
# 				client.send(message.encode("utf-8"))
# 			case _:
# 				client.send(f"~ > Invalid command: '{message}'\n".encode("utf-8"))

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
				response = bytes.fromhex("55aa01000000000030003001")
				client.send(response)
				LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))

			# Case: Start Enrollment COMMAND
			case ['22', '00']:
				LOG.info("CASE: ['22', '00']")

				# Test Case "fingerprint module DataBase is full":
				# If the (clock) minute is odd -> send to the microcontroller (client) a response with the corresponding
				# error message -> NACK_DB_IS_FULL: 0x10 09
				now = datetime.datetime.now()
				minute = now.minute
				if minute % 2 != 0:
					response = bytes.fromhex("55aa01000910000031004a01")
					client.send(response)
					LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))
				else:
					# Testcase: For this command the Parameter is the ID of the FingerPrint
					#           If the ID is outside the [DEC][0 - 19] -> [0 13][HEX] range, return error NACK_INVALID_POS
					#           If the ID is "1 to 7", return error NACK_IS_ALREADY_USED
					if int(message_hex[4:5]) < 0 or int(message_hex[4:5]) > 13:
						response = bytes.fromhex("55aa01000310000031004401")
						client.send(response)
						LOG.info("COMMAND sent: {}".format(' '.join('{:02x}'.format(x) for x in response)))
					elif 0 < int(message_hex[4:5]) < 7:
						response = bytes.fromhex("55aa01000510000031004601")
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

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import sys
import time
import json
import serial

arduino = serial.Serial('COM0', 115200) # New serial connection. Change port to your Arduino's Port

def sendBombStatus(bomb, ctrl): # Send bomb status to arduino
	if(bomb == 'defused'):
		arduino.write('bd')
	elif(bomb == 'planted'):
		arduino.write('bp')
	elif(bomb == 'exploded'):
		arduino.write('be')
	else:
		arduino.write('bn')
	if(ctrl == 0):
		print('bomb while')
		while(arduino.read(1) != 'b'):
			sendBombStatus(bomb, 1)
	return None
	
def sendHealthStatus(health, ctrl): # Send health status to arduino
	if health == 100:
		arduino.write('h')
		arduino.write(str(health))
	elif health < 100 and health >= 10: #thanks to Bjorn Braucker
		arduino.write('h0')
		arduino.write(str(health))
	elif health < 10 and health >= 0:
		arduino.write('h00')
		arduino.write(str(health))
	if(ctrl == 0):
		print('health while')
		while(arduino.read(1) != 'h'):
			sendHealthStatus(health, 1)
	return None
	
def sendAmmoStatus(ammo, ctrl): # Send ammo status to arduino
	if ammo != None:
		arduino.write('a')
		arduino.write(str(ammo))
		if(ctrl == 0):
			print('ammo while')
			while(arduino.read(1) != 'a'):
				sendAmmoStatus(ammo, 1)
	else:
		arduino.write('a')
		arduino.write(str(0))
		if(ctrl == 0):
			print('ammo while')
			while(arduino.read(1) != 'a'):
				sendAmmoStatus(ammo, 1)
	return None
	
class MyServer(HTTPServer):
	def init_state(self):
		"""
		You can store states over multiple requests in the server
		"""
		self.player_health = None
		self.bomb_state = None
		self.ammo_status = None


class MyRequestHandler(BaseHTTPRequestHandler):
	def do_POST(self):
		length = int(self.headers['Content-Length'])
		body = self.rfile.read(length).decode('utf-8')

		self.parse_payload(json.loads(body))

		self.send_header('Content-type', 'text/html')
		self.send_response(200)
		self.end_headers()

	def parse_payload(self, payload):
		if arduino.in_waiting > 0:
			fromArduino = arduino.read(arduino.in_waiting)
			print(fromArduino)
		player_health = self.get_player_health(payload)
		bomb_state = self.get_bomb_state(payload)
		ammo_status = self.get_ammo_status(payload)
		
		if bomb_state != self.server.bomb_state:
			self.server.bomb_state = bomb_state
			sendBombStatus(bomb_state, 0)
			print('Bomb state: %s' % bomb_state)
		
		if player_health != self.server.player_health:
			self.server.player_health = player_health
			sendHealthStatus(player_health, 0)
			print('Player health: %s' % player_health)
			
		if ammo_status != self.server.ammo_status:
			self.server.ammo_status = ammo_status
			sendAmmoStatus(ammo_status, 0)
			print('Ammo status: %s' % ammo_status)
			
			
	def get_player_health(self, payload):
		if 'player' in payload and 'state' in payload['player'] and 'health' in payload['player']['state']:
			return payload['player']['state']['health']
		else:
			return None
   
	def get_bomb_state(self, payload):
		if 'round' in payload and 'bomb' in payload['round']:
			return payload['round']['bomb']
		else:
			return None
   
	def get_ammo_status(self, payload): # if ammo under 40 percent send 1 else 0
		if 'player' in payload and 'weapons' in payload['player']:
			if 'weapon_1' in payload['player']['weapons'] and 'state' in payload['player']['weapons']['weapon_1']:
				if payload['player']['weapons']['weapon_1']['state'] == 'active':
					if 'ammo_clip' in payload['player']['weapons']['weapon_1'] and 'ammo_clip_max' in payload['player']['weapons']['weapon_1']:
						if payload['player']['weapons']['weapon_1']['ammo_clip'] < (((payload['player']['weapons']['weapon_1']['ammo_clip_max']) * 40) / 100):
							return 1;
						else:
							return 0;
			if 'weapon_2' in payload['player']['weapons'] and 'state' in payload['player']['weapons']['weapon_2']:
				if payload['player']['weapons']['weapon_2']['state'] == 'active':
					if 'ammo_clip' in payload['player']['weapons']['weapon_2'] and 'ammo_clip_max' in payload['player']['weapons']['weapon_2']:
						if payload['player']['weapons']['weapon_2']['ammo_clip'] < (((payload['player']['weapons']['weapon_2']['ammo_clip_max']) * 40) / 100):
							return 1;
						else:
							return 0;
			if 'weapon_3' in payload['player']['weapons'] and 'state' in payload['player']['weapons']['weapon_3']:
				if payload['player']['weapons']['weapon_3']['state'] == 'active':
					if 'ammo_clip' in payload['player']['weapons']['weapon_3'] and 'ammo_clip_max' in payload['player']['weapons']['weapon_3']:
						if payload['player']['weapons']['weapon_3']['ammo_clip'] < (((payload['player']['weapons']['weapon_3']['ammo_clip_max']) * 40) / 100):
							return 1;
						else:
							return 0;
			else:
				return None
		else:
			return None

	def log_message(self, format, *args):
		#Prevents requests from printing into the console
		return


server = MyServer(('localhost', 3001), MyRequestHandler) # make sure cfg's and this port is same

server.init_state()

try:
 server.serve_forever()
except (KeyboardInterrupt, SystemExit):
 pass

server.server_close()

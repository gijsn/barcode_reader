#!/bin/python3

def num2bin(n):
  p = f'{n:b}'.count('1')%2
  b = (n<<3) | (p << 2) | ((p^1) <<1) | 1
  return b

def bin2str(b):
  return f'{b:013b}'

def num2str(b):
  return bin2str(num2bin(i))

def bin2code(b):
  return bin2str(b).replace('0', '█  ').replace('1', '██ ')

def num2code(n):
  return bin2code(num2bin(n))

for i in range(256):
  print(f'{i} - {num2str(i)}\n{f'{num2code(i)}\n'*3}\n\n')

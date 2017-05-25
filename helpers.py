# -*- coding: utf-8 -*-


import unicodedata
import re
from variables import *


def remove_accents(s):
   return ''.join(c for c in unicodedata.normalize('NFD', s)
                  if unicodedata.category(c) != 'Mn')


def size_room(x):
    if 50<=x:
        return 'Aula_Grande'
    elif 25<x<50:
        return 'Aula_Normal'
    else:
        return 'Aula_Pequena'


def hours_slot(i):
    j = int(i)
    if j<= 2:
        return GRUPOS[0]
    elif 2<= j <= 5:
        return GRUPOS[1]
    else:
        return GRUPOS[2]


def time_class(s):
   t = s.replace(',','.')
   return float(t) if t else float(0)

def normalize_name(s):
   p = re.compile("\(([GC][0-9]+)\)|\(\*\)")
   q = re.compile("\([GC][0-9]+\)")
   if q.match(s):
      return p.sub("", s).upper().strip(), q.match(s).group()[1:-1]
   else:
      return p.sub("", s).upper().strip(), ''

def expected_number_students(array):
   maximum = 0
   for i in array:
      num =i.strip()
      if num:
         maximum = max(int(num), maximum)
   return maximum
def name_degree(l):
   temp = l.split('_')
   if len(temp)>1:
      l0, year = temp
   else:
      l0, year = temp[0],''
   s = normalize_name(l0)[0]
   return s, year

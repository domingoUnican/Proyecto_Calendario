import xml.etree.ElementTree as etree
import datetime
from math import ceil
from helpers import *
from variables import *

def csv2xml(semester=2):

    # Variables that will contain our data
    rooms = dict()
    asigs = dict()

    with open(ROOMS, encoding='utf-8') as rooms_file:
        for line in rooms_file:
            contents = line.split(';')
            name = contents[0].strip().upper()
            rooms[name] = int(contents[1])

    with open(ASIGS, encoding='utf-8') as subjects_file:
        for line in subjects_file
        
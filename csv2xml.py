import xml.etree.ElementTree as etree
import datetime
from math import ceil
from helpers import *
from variables import *

def csv2xml(semester=2):
    rooms, subjects, profs, subject_profs = fetch(semester)

def fetch(semester):
    # Variables that will contain our data
    rooms = dict()
    subjects = dict()
    profs = dict()
    subject_profs = dict()

    with open(ROOMS, encoding='utf-8') as rooms_file:
        for line in rooms_file:
            contents = line.split(';')
            name = contents[0].strip().upper()
            rooms[name] = int(contents[1])

    with open(ASIGS, encoding='utf-8') as subjects_data:
        for line in subjects_data
            contents = line.split(';')
            subject_name = remove_accents(contents[0])
            subject_data = dict()
            subject_data['course'] = 
            subject_data['lab_hours'] = 
            subject_data['lab_groups'] = 
            subjects[name] = subject_data

    for subject_profs_sheet in PROFS
        with open(subject_profs_sheet) as subject_profs_data
            for line in subject_profs_data
                subject_profs[]

        
import xml.etree.ElementTree as etree
import datetime
from math import ceil
from helpers import *
from variables import *


def csv2xml(semester=2):
    rooms, subjects, profs, subject_profs = fetch(semester)

    ## METADATA ##
    root = etree.Element('HighSchoolTimetableArchive', Id='HORARIO_FACULTAD_CIENCIAS')
    doc = etree.ElementTree(root)
    instances = etree.SubElement(root, 'Instances')
    instance = etree.SubElement(instances, 'Instance', Id='V0.1')
    metadata = etree.SubElement(instance, 'MetaData')
    name = etree.SubElement(metadata, 'Name')
    name.text = 'Horario de la facultad de Ciencias'
    contributor = etree.SubElement(metadata, 'Contributor')
    contributor.text = 'Mario Meissner and Domingo Gomez'
    today = etree.SubElement(metadata, 'Date')
    today.text = datetime.datetime.now().__str__()
    country = etree.SubElement(metadata, 'Country')
    country.text = 'Spain'
    Description = etree.SubElement(metadata, 'Description')
    Description.text = 'Facultad de ciencias'

    ## TIMES ##
    times = etree.SubElement(instance, 'Times')

    # Defining time groups: todas_horas, antes_descanso, despues_descanso y tarde
    time_groups = etree.SubElement(times, 'TimeGroups')
    todas_horas = etree.SubElement(time_groups, 'TimeGroup', Id="TodasHoras")
    name = etree.SubElement(todos, 'Name')
    name.text = "Todas las horas"

    for grupo in TIME_GROUPS:
        group = etree.SubElement(tg, 'TimeGroup', Id=grupo)
        name = etree.SubElement(group, 'Name')
        name.text = TIME_GROUPS_DESCRIPTION[group]
    
    #Defining the days, lunes to viernes
    for day in DAYS:
        day_element = etree.SubElement(time_groups, 'Day', Id=day)
        name = etree.SubElement(day_element, 'Name')
        name.text = day
    
    # Defining the hours each day has
    for hour in HOURS:
        for day in DAYS:
            time = etree.SubElement(times, 'Time', Id=day + hour)
            name = etree.SubElement(time, 'Name')
            name.text = day + ' - ' + hour
            day_reference = etree.SubElement(time, 'Day', Reference=day)
            time_groups_reference = etree.SubElement(time, 'TimeGroups')
            etree.SubElement(time_groups_reference, 'TimeGroup', Reference=hours_slot(hour))
            etree.SubElement(time_groups_reference, 'TimeGroup', Reference="TodasHoras")


def fetch(semester):
    # Variables that will contain our data
    rooms = dict()          # [roomName] -> capacity
    subjects = dict()       # [degree][subjectName] -> subject_data
    profs = dict()          # [professorName] -> professor_data
    subject_profs = dict()  # [degree][subjectName] -> professor_list

    with open(ROOMS, encoding='utf-8') as rooms_file:
        for line in rooms_file:
            contents = line.split(';')
            name = contents[0].strip().upper()
            rooms[name] = int(contents[1])

    with open(SUBJ, encoding='utf-8') as subjects_data:
        for subject in subjects_data:
            subject = subject.split(';')
            degrees = subject[6].upper().split(',')
            subject_name = remove_accents(subject[0])
            subject_data = dict()
            subject_data['course'] = int(subject[1])
            subject_data['lab_hours'] = int(subject[3])
            subject_data['num_lab_groups'] = int(subject[5])
            # We add the subject to each degree it is taught at
            for degree in DEGREES:
                if degree in degrees:
                    subjects[degree][subject_name] = subject_data

    for degree_number, subject_profs_sheet in enumerate(PROFS):
        degree_name = DEGREES[degree_number]
        with open(subject_profs_sheet) as subject_profs_data:
            for subject in subject_profs_data:
                subject_name = remove_accents(subject[2])
                professor_data = dict()
                # TODO
                subject_profs[degree_name][subject].append(professor_data)

    return rooms, subjects, profs, subject_profs




if __name__ == '__main__':
    csv2xml(2)

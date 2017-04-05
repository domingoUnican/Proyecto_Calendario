# -*- coding: utf-8 -*-


import os.path
import xml.etree.ElementTree as etree
import datetime
from math import ceil

DIR = os.path.join('.','datos')
PROFS = os.path.join(DIR, 'profesores.csv')
ROOMS = os.path.join(DIR, 'aulas.csv')
ASIG = os.path.join(DIR, 'informatica_1.csv')
OUT = os.path.join(DIR, 'outfile3.xml')
DIAS = ['Lunes', 'Martes', 'Miercoles', 'Jueves', 'Viernes']
GRUPOS = ['AntesDescanso', 'DespuesDescanso','Tarde']
HORAS = [str(i) for i in range(1,3)]
TRANS = {str(i): GRUPOS[(i-1)//3] for i in range(1,9)}
RECURSOS = ['Teacher', 'Room', 'Class']
RECURSOS_NAME = {
    'Teacher': 'Profesores',
    'Room': 'Aulas',
    'Class': 'Cursos'
}
TYPES_ROOMS = ['Grandes', 'Normales', 'Laboratorios', 'Pequenas']
CLASSES = ['Curso_'+str(i)+str(j) for i in range(1,5) for j in range(1,2)]
SEMESTERS = ['Cuatrimestre_1', 'Cuatrimestre_2']
SEMESTER = '1'
def size_room(x):
    if 50<=x:
        return 'Grandes'
    elif 25<x<50:
        return 'Normales'
    else:
        return 'Pequenas'


class Csv2Xml(object):
    """
    Documentation:

    This is a simple class to convert the data in csv to the xml format.

    """
    def __init__(self, profs=PROFS, rooms=ROOMS, asig=ASIG, out=OUT):
        self.asig = dict()
        self.prof = dict()
        self.rooms = dict()
        hours = slice(7, 12)
        with open(ASIG, encoding='utf-8') as f:
            for line in f:
                contents = line.split(';')
                self.asig[contents[2]] = [0, 0, contents[0], contents[1]]
                self.asig[contents[2]] += [int(i.strip()) for i in contents[3:]
                                           if i.strip()]
        with open(PROFS, encoding='utf-8') as f:
            hours = slice(2, 5, 2)
            for line in f:
                contents = line.split(';')
                n_hours = sum([int(i) for i in contents[hours]])
                if contents[0] not in self.asig:
                    self.asig[contents[0]] = [0, 0, *contents[hours], 0]
                self.asig[contents[0]][0] += (int(contents[7]) +
                                              int(contents[8]))
                self.asig[contents[0]][1] += int(contents[9])
                if contents[5] not in self.prof:
                    self.prof[contents[5]] = (contents[0], n_hours)
                else:
                    asigs, c = self.prof[contents[5]]
                    c += n_hours
                    self.prof[contents[5]] = (', '.join([asigs, contents[0]]),
                                              c)
        with open(ROOMS, encoding='utf-8') as f:
            for line in f:
                contents = line.split(';')
                self.rooms[contents[0]] = int(contents[1])

    def toXHSTT(self, out=OUT, iden=ASIG):
        root = etree.Element('HighSchoolTimetableArchive', Id=iden)
        doc = etree.ElementTree(root)
        instances = etree.SubElement(root, 'Instances')
        # TODO: Change the Id of the instance, maybe it is necessary to put
        #       a instance for each degree
        instance = etree.SubElement(instances, 'Instance', Id=iden)
        metadata = etree.SubElement(instance, 'MetaData')
        name = etree.SubElement(metadata, 'Name')
        name.text = 'Horario de Informatica'
        contributor = etree.SubElement(metadata, 'Contributor')
        contributor.text = 'Jose Ramon and Mario'
        today = etree.SubElement(metadata, 'Date')
        today.text = datetime.datetime.now().__str__()
        country = etree.SubElement(metadata, 'Country')
        country.text = 'Spain'
        Description = etree.SubElement(metadata, 'Description')
        Description.text = 'Unican'
        # Until here, we have just defined the metadata, now the times
        times = etree.SubElement(instance, 'Times')
        tg = etree.SubElement(times, 'TimeGroups')
        # Here is defined all the days
        for dia in DIAS:
            day = etree.SubElement(tg, 'Day', Id=dia)
            name = etree.SubElement(day,'Name')
            name.text = dia
        todos = etree.SubElement(tg, 'TimeGroup', Id="TodasHoras")
        name = etree.SubElement(todos,'Name')
        name.text = "Todas las horas"

        # We also make three different groups, this must be necessary
        # to make professors happy. So they do not have teaching
        # before noon and after noon
        for grupo in GRUPOS:
            group = etree.SubElement(tg, 'TimeGroup', Id=grupo)
            name = etree.SubElement(group, 'Name')
            name.text = grupo
        for hora in HORAS:
            for dia in DIAS:
                time = etree.SubElement(times, 'Time', Id=dia+hora)
                name = etree.SubElement(time, 'Name')
                name.text = dia + ' - ' + hora
                day = etree.SubElement(time, 'Day', Reference=dia)
                tg = etree.SubElement(time, 'TimeGroups')
                etree.SubElement(tg, 'TimeGroup', Reference=TRANS[hora])
                etree.SubElement(tg, 'TimeGroup', Reference="TodasHoras")
        # Now, we define the resources types
        resources = etree.SubElement(instance, 'Resources')
        resources_types = etree.SubElement(resources, 'ResourceTypes')
        for recurso in RECURSOS:
            resource = etree.SubElement(resources_types, 'ResourceType',
                                        Id=recurso)
            name = etree.SubElement(resource, 'Name')
            name.text = RECURSOS_NAME[recurso]
        # Here are the resource groups: type_rooms, semeters y classes,
        # gr_profesores, gr_clases, gr_aulas
        resources_groups = etree.SubElement(resources, 'ResourceGroups')
        for recurso in RECURSOS:
            resource = etree.SubElement(resources_groups, 'ResourceGroup',Id='gr_' + recurso)
            name = etree.SubElement(resource, 'Name')
            name.text = RECURSOS_NAME[recurso]
            etree.SubElement(resource, 'ResourceType', Reference=recurso)
        for tipo in TYPES_ROOMS:
            resource_group = etree.SubElement(resources_groups, 'ResourceGroup',
                                              Id=tipo)
            name = etree.SubElement(resource_group, 'Name')
            name.text = tipo
            etree.SubElement(resource_group, 'ResourceType',Reference='Room')
        for curso in CLASSES:
            resource_group = etree.SubElement(resources_groups, 'ResourceGroup',
                                              Id=curso)
            name = etree.SubElement(resource_group, 'Name')
            name.text = curso
            etree.SubElement(resource_group, 'ResourceType',Reference='Class')
        # Now, we define the resources, we start with the teachers
        for prof in self.prof:
            resource = etree.SubElement(resources, 'Resource',
                                        Id=prof.replace(' ','_'))
            name = etree.SubElement(resource, 'Name')
            name.text = prof
            resource_type = etree.SubElement(resource, 'ResourceType',
                                             Reference="Teacher")
            resource_groups = etree.SubElement(resource, 'ResourceGroups')
            resource_group = etree.SubElement(resource_groups, 'ResourceGroup',
                                              Reference = 'gr_Teacher')
        # Here we define the different rooms
        for room in self.rooms:
            resource = etree.SubElement(resources, 'Resource',
                                        Id=room)
            name = etree.SubElement(resource, 'Name')
            name.text = room
            resource_type = etree.SubElement(resource, 'ResourceType',
                                             Reference="Room")
            resource_groups = etree.SubElement(resource, 'ResourceGroups')
            etree.SubElement(resource_groups, 'ResourceGroup',
                             Reference=size_room(self.rooms[room]))
            etree.SubElement(resource_groups, 'ResourceGroup',
                             Reference='gr_Room')
            # if the room is a lab,
            if 'ab' in room:
                etree.SubElement(resource_groups, 'ResourceGroup',
                                 Reference='Laboratorios')
        # Now, the different courses. We will have to add three different groups
        # for each course. If this is
        for curso in CLASSES:
            resource = etree.SubElement(resources, 'Resource',
                                        Id=curso)
            name = etree.SubElement(resource, 'Name')
            name.text = curso
            resource_type = etree.SubElement(resource, 'ResourceType',
                                             Reference="Class")
            resource_groups = etree.SubElement(resource, 'ResourceGroups')
            etree.SubElement(resource_groups, 'ResourceGroup',
                             Reference='gr_Class')
        # Now, the events, for now, we suppose that the number of hours is evenly
        # explict in all 15 weeks. Labs are 2 hours per week, so each 30 hours means
        # another group.
        events = etree.SubElement(instance, 'Events')
        event_groups = etree.SubElement(events, 'EventGroups')
        for asign in self.asig:
            course_asign = etree.SubElement(event_groups, 'Course',
                                            Id=asign.replace(' ', '_'))
            name = etree.SubElement(course_asign, 'Name')
            name.text = asign
        all_events = etree.SubElement(event_groups, 'EventGroup',
                                            Id='gr_AllEvents')
        name = etree.SubElement(all_events, 'Name')
        name.text = 'Todos los cursos'
        for asign in self.asig:
            # we have to separate theory and laboratories, so we have to consider
            # laboratories plus
            if not asign:
                continue
            event_t = etree.SubElement(events, 'Event', Id=asign.replace(' ','_')+'_t')
            name = etree.SubElement(event_t, 'Name')
            name.text = asign + ': Teoria'
            duration = etree.SubElement(event_t, 'Duration')
            duration.text = str(int(ceil(float(self.asig[asign][0])/15.0)))
            resources = etree.SubElement(event_t, 'Resources')
            courses = [i for i in CLASSES
                       if 'Curso_'+self.asig[asign][2] in i]
            # In the theory lessons, all groups attends
            for course in courses:
                resource = etree.SubElement(resources, 'Resource',
                                            Reference=course)
            # All professors in a lecture must assists to all the lectures
            list_prof = [p for p in self.prof if asign in self.prof[p]]
            for p in list_prof:
                resource = etree.SubElement(resources, 'Resource',
                                            Reference=p.replace(' ', '_'))
            resource = etree.SubElement(resources, 'Resource')
            role = etree.SubElement(resource, 'Role')
            role.text = 'Room'
            etree.SubElement(resource, 'ResourceType',
                                             Reference = 'Room')
            event_groups = etree.SubElement(event_t, 'EventGroups')
            etree.SubElement(event_groups, 'EventGroup', Reference='gr_AllEvents')
            etree.SubElement(event_groups, 'EventGroup',
                             Reference=asign.replace(' ','_'))

        constraints = etree.SubElement(instance, 'Constraints')
        assign_resource = etree.SubElement(constraints,
                                          'AssignResourceConstraint',
                                          Id='Aulas')
        name = etree.SubElement(assign_resource, 'Name')
        name.text = 'Asignar aulas'
        required = etree.SubElement(assign_resource, 'Required')
        required.text = 'true'
        weight = etree.SubElement(assign_resource, 'Weight')
        weight.text = '1'
        CostFunction = etree.SubElement(assign_resource, 'CostFunction')
        CostFunction.text = 'Linear'
        AppliesTo = etree.SubElement(assign_resource, 'AppliesTo')
        event_groups = etree.SubElement(AppliesTo, 'EventGroups')
        etree.SubElement(event_groups, 'EventGroup', Reference='gr_AllEvents')
        role = etree.SubElement(assign_resource, 'Role')
        role.text = 'Room'
        avoid = etree.SubElement(constraints, 'AvoidClashesConstraint', Id='Choques')
        name = etree.SubElement(avoid, 'Name')
        name.text = 'Choques'
        required = etree.SubElement(avoid, 'Required')
        required.text = 'true'
        weight = etree.SubElement(avoid, 'Weight')
        weight.text = '1'
        CostFunction = etree.SubElement(avoid, 'CostFunction')
        CostFunction.text = 'Linear'
        AppliesTo = etree.SubElement(avoid, 'AppliesTo')
        resource_groups = etree.SubElement(AppliesTo, 'ResourceGroups')
        for recurso in RECURSOS:
            resource = etree.SubElement(resource_groups, 'ResourceGroup',Reference='gr_' + recurso)
        f = open(OUT,'wb')
        doc.write(f)

if __name__ == '__main__':
    c = Csv2Xml()
    c.toXHSTT()
    print("Hecho")

# -*- coding: utf-8 -*-


import os.path
import xml.etree.ElementTree as etree
import datetime
from math import ceil
from helpers import *
from variables import *


class Csv4Xml(object):
    """
    Documentation:

    This is a simple class to convert the data in csv to the xml format.

    """
    def __init__(self, profs=PROFS, rooms=ROOMS,
                 asig=ASIG, out=OUT, semester='2'):
        self.asig = dict()
        self.prof = dict()
        self.rooms = dict()
        self.codes = dict()
        CLASSES_NAME.clear()
        DEGREES.clear()
        N_GROUPS = {str(i):1 for i in range(1,5)}
        with open(ROOMS, encoding='utf-8') as f:
            for line in f:
                contents = line.split(';')
                name = contents[0].strip().upper()
                self.rooms[name] = int(contents[1])
        for fich_asig in ASIG:
            with open(fich_asig, encoding='utf-8') as f:
                for line in f:
                    contents = line.split(';')
                    if contents[2]==semester:
                        name = normalize_name(contents[0])[0]
                        self.asig[name] = contents[1:-1]
                        self.asig[name].append([])
                        for temp in contents[6].split(','):
                            t_n = name_degree(temp)
                            for temp20 in contents[7].split(','):
                                temp2 = normalize_name(temp20)[0]
                                self.asig[name][-1].append((contents[1],t_n,
                                                        temp2))
                                if (contents[1],t_n,temp2) not in DEGREES:
                                    DEGREES.append((contents[1],t_n,temp2))

                        N_GROUPS[contents[1]] = max(int(contents[5]),
                                                    N_GROUPS[contents[1]])
                        # Todos los cursos tienen el mismo numero de grupos
                        # Las asignaturas tendran tantas horas como grupos.
        for i, degree, special in DEGREES:
            for j in range(1,N_GROUPS[i]+1):
                key = 'Class%s_%s_%s%s'%(str(i),degree,special,str(j))
                key = key.replace(' ','_')
                if not special:
                    value = 'Clase %s (grupo %s) de %s'%(str(i),str(j),degree)
                else:
                    value = 'Clase %s (grupo %s) de %s (%s)'%(str(i),
                                                              str(j),
                                                              degree,
                                                              special)
                CLASSES_NAME[key] = value
        CLASSES.clear()
        for temp in CLASSES_NAME.keys():
            CLASSES.append(temp)
        CLASSES.sort()
        for fich_prof in PROFS:
            with open(fich_prof, encoding='utf-8') as f:
                hours = slice(8, 10)
                for line in f:
                    contents = line.split(';')
                    code = normalize_name(contents[0])[0]
                    if code in self.codes:
                        n_name = self.codes[code]
                    else:
                        n_name =  normalize_name(contents[1])[0]
                    if contents[5] == semester:
                        if n_name not in self.asig:
                            continue
                        n_prof = normalize_name(contents[6])[0]
                        if (contents[2] == 'O' and
                            self.asig[n_name][-2] in ('No','','Optativa')):
                            self.asig[n_name][-2] = 'O'

                        n_hours_class = sum([time_class(i)
                                     for i in contents[hours]])
                        n_hours_lab = time_class(contents[10])
                        if n_prof not in self.prof:
                            self.prof[n_prof] = (n_name,
                                                 n_hours_class + n_hours_lab)
                        else:
                            asigs, c = self.prof[n_prof]
                            c += n_hours_class + n_hours_lab
                            asigs, c = self.prof[n_prof]
                            if n_name not in asigs:
                                c += n_hours_class + n_hours_lab
                                asigs = ','.join([asigs,n_name])
                            self.prof[n_prof] = (asigs, c)

    def toXHSTT(self, out=OUT, iden=ASIG):
        root = etree.Element('HighSchoolTimetableArchive', Id='TODO')
        doc = etree.ElementTree(root)
        instances = etree.SubElement(root, 'Instances')
        # TODO: Change the Id of the instance, maybe it is necessary to put
        #       a instance for each degree
        instance = etree.SubElement(instances, 'Instance', Id='TODO')
        metadata = etree.SubElement(instance, 'MetaData')
        name = etree.SubElement(metadata, 'Name')
        name.text = 'Horario de Informatica'
        contributor = etree.SubElement(metadata, 'Contributor')
        contributor.text = 'Jose Ramon, Mario and Domingo'
        today = etree.SubElement(metadata, 'Date')
        today.text = datetime.datetime.now().__str__()
        country = etree.SubElement(metadata, 'Country')
        country.text = 'Spain'
        Description = etree.SubElement(metadata, 'Description')
        Description.text = 'Facultad de ciencias'
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
            name.text = GRUPOS_NAMES[grupo]
        for hora in HORAS:
            for dia in DIAS:
                time = etree.SubElement(times, 'Time', Id=dia+hora)
                name = etree.SubElement(time, 'Name')
                name.text = dia + ' - ' + hora
                day = etree.SubElement(time, 'Day', Reference=dia)
                tg = etree.SubElement(time, 'TimeGroups')
                etree.SubElement(tg, 'TimeGroup', Reference=hours_slot(hora))
                etree.SubElement(tg, 'TimeGroup', Reference="TodasHoras")
        # Now, we define the resources types
        resources = etree.SubElement(instance, 'Resources')
        resources_types = etree.SubElement(resources, 'ResourceTypes')
        for recurso in RECURSOS:
            resource = etree.SubElement(resources_types, 'ResourceType',
                                        Id=recurso)
            name = etree.SubElement(resource, 'Name')
            name.text = RECURSOS_NAME[recurso]
        for type_aulas in TYPES_ROOMS:
            resource = etree.SubElement(resources_types, 'ResourceType',
                                        Id=type_aulas)
            name = etree.SubElement(resource, 'Name')
            name.text = TYPES_ROOMS_NAMES[type_aulas]

        # Here are the resource groups: type_rooms, semeters y classes,
        # gr_profesores, gr_clases, gr_aulas
        resources_groups = etree.SubElement(resources, 'ResourceGroups')
        for recurso in RECURSOS:
            resource = etree.SubElement(resources_groups, 'ResourceGroup',
                                        Id='gr_' + recurso)
            name = etree.SubElement(resource, 'Name')
            name.text = RECURSOS_NAME[recurso]
            etree.SubElement(resource, 'ResourceType', Reference=recurso)
        for tipo in TYPES_ROOMS:
            resource_group = etree.SubElement(resources_groups, 'ResourceGroup',
                                              Id='gr_'+tipo)
            name = etree.SubElement(resource_group, 'Name')
            name.text = TYPES_ROOMS_NAMES[tipo]
            etree.SubElement(resource_group, 'ResourceType',
                             Reference=TYPES_ROOMS[tipo])
        for curso in CLASSES:
            resource_group = etree.SubElement(resources_groups, 'ResourceGroup',
                                              Id=curso)
            name = etree.SubElement(resource_group, 'Name')
            name.text = CLASSES_NAME[curso]
            etree.SubElement(resource_group, 'ResourceType',Reference='Class')
        # Now, we define the resources, we start with the teachers
        for prof in self.prof:
            resource = etree.SubElement(resources, 'Resource',
                                        Id=remove_accents(prof.replace(' ','_')))
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
                                        Id=remove_accents(room.replace(' ','_')))
            name = etree.SubElement(resource, 'Name')
            name.text = room
            resource_type = etree.SubElement(resource, 'ResourceType',
                                             Reference="Room" if 'ABO' not in room else 'Laboratory')
            resource_groups = etree.SubElement(resource, 'ResourceGroups')
            etree.SubElement(resource_groups, 'ResourceGroup',
                             Reference='gr_'+size_room(self.rooms[room]))
            etree.SubElement(resource_groups, 'ResourceGroup',
                             Reference='gr_Room')
            # if the room is a lab,
            if 'LABORATORIO' in room:
                etree.SubElement(resource_groups, 'ResourceGroup',
                                 Reference='gr_Laboratorio')
        # Now, the different courses. We will have to add three different groups
        # for each course. If this is
        for curso in CLASSES:
            resource = etree.SubElement(resources, 'Resource',
                                        Id=remove_accents(curso))
            name = etree.SubElement(resource, 'Name')
            name.text = CLASSES_NAME[curso]
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
                                            Id=remove_accents(asign.replace(' ', '_')))
            name = etree.SubElement(course_asign, 'Name')
            name.text = remove_accents(asign)
        all_events = etree.SubElement(event_groups, 'EventGroup',
                                            Id='gr_AllEvents')
        name = etree.SubElement(all_events, 'Name')
        name.text = 'Todos los cursos'
        # We add the theory of all courses
        for asign in self.asig:
            # we have to separate theory and laboratories, so we have to consider
            # laboratories plus
            if int(self.asig[asign][3])< 1:
                continue
            event_t = etree.SubElement(events, 'Event',
                                       Id=remove_accents(asign.replace(' ','_'))+'_t')
            name = etree.SubElement(event_t, 'Name')
            name.text = remove_accents(asign) + ': Teoria'
            duration = etree.SubElement(event_t, 'Duration')
            duration.text = self.asig[asign][3]
            etree.SubElement(event_t, 'Course',
                             Reference=remove_accents(asign.replace(' ', '_')))
            resources = etree.SubElement(event_t, 'Resources')
            courses = []
            for temp_course in range(1,int(N_GROUPS[self.asig[asign][0]])+1):
                for i,degree,special in self.asig[asign][-1]:
                    key = 'Class%s_%s_%s%s'%(str(i),degree,special,str(temp_course))
                    key = key.replace(' ','_')
                    courses.append(key)
            # In the theory lessons, all groups attends
            for course in courses:
                resource = etree.SubElement(resources, 'Resource',
                                            Reference=course)
            # All professors in a lecture must assists to all the lectures
            list_prof = [p for p in self.prof if asign in self.prof[p][0].split(',')]
            for p in list_prof:
                resource = etree.SubElement(resources, 'Resource',
                                            Reference=remove_accents(p.replace(' ', '_')))
            resource = etree.SubElement(resources, 'Resource')
            role = etree.SubElement(resource, 'Role')
            role.text = 'Room'
            etree.SubElement(resource, 'ResourceType',  Reference = 'Room')
            event_groups = etree.SubElement(event_t, 'EventGroups')
            etree.SubElement(event_groups, 'EventGroup', Reference='gr_AllEvents')
        # This is to generate the labs
        for asign in self.asig:
            for temp_i in range(1,N_GROUPS[self.asig[asign][0]]+1):
                # we have to separate theory and laboratories, so we have to consider
                # laboratories plus
                if int(self.asig[asign][2]) <1:
                    continue
                event_t = etree.SubElement(events, 'Event',
                                           Id=remove_accents(asign.replace(' ','_'))+'_l'+str(temp_i))
                name = etree.SubElement(event_t, 'Name')
                name.text = remove_accents(asign) + ': Laboratorio'
                duration = etree.SubElement(event_t, 'Duration')
                duration.text = self.asig[asign][2]
                etree.SubElement(event_t, 'Course',
                                 Reference=remove_accents(asign.replace(' ', '_')))
                resources = etree.SubElement(event_t, 'Resources')
                courses = []
                for i,degree,special in self.asig[asign][-1]:
                    key = 'Class%s_%s_%s%s'%(str(i),degree,special,str(temp_i))
                    key = key.replace(' ','_')
                    courses.append(key)
                # In the lab sessions, only one group attends
                for course in courses:
                    resource = etree.SubElement(resources, 'Resource',
                                                Reference=course)
                # All professors in a lecture must assists to all the lectures
                list_prof = [p for p in self.prof if asign in self.prof[p][0].split(',')]
                for p in list_prof:
                    resource = etree.SubElement(resources, 'Resource',
                                                Reference=remove_accents(p.replace(' ', '_')))
                resource = etree.SubElement(resources, 'Resource')
                role = etree.SubElement(resource, 'Role')
                role.text = 'Room'
                etree.SubElement(resource, 'ResourceType',
                                                 Reference = 'Laboratory')
                event_groups = etree.SubElement(event_t, 'EventGroups')
                etree.SubElement(event_groups, 'EventGroup', Reference='gr_AllEvents')

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
        prefer = etree.SubElement(constraints, 'PreferTimesConstraint', Id='manana')
        name = etree.SubElement(prefer, 'Name')
        name.text = 'Todo por la mañana'
        required = etree.SubElement(prefer, 'Required')
        required.text = 'false'
        weight = etree.SubElement(prefer, 'Weight')
        weight.text = '1'
        CostFunction = etree.SubElement(prefer, 'CostFunction')
        CostFunction.text = 'Linear'
        AppliesTo = etree.SubElement(prefer, 'AppliesTo')
        event_groups = etree.SubElement(AppliesTo, 'EventGroups')
        event_group = etree.SubElement(event_groups, 'EventGroup',
                                       Reference='gr_AllEvents'
        )
        times_groups = etree.SubElement(prefer, 'TimeGroups')
        times_group = etree.SubElement(times_groups, 'TimeGroup',
                                       Reference='AntesDescanso'
        )
        times_group = etree.SubElement(times_groups, 'TimeGroup',
                                       Reference='DespuesDescanso'
        )
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
        doc.write(f, encoding = 'UTF-8')
        f.close()
        f = open(OUT,'r')
        salida = f.read()
        salida = salida.replace('><','>\n<')
        f.close()
        f = open(OUT,'w')
        f.write(salida)
        f.close()
if __name__ == '__main__':
    c = Csv4Xml()
    c.toXHSTT()
    print("Hecho")

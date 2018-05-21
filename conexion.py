from helpers import dedupe
from ParserSQL_233 import *
from ParserSQLNew import *

class con_bd:

    def all_prof_id(self):
        '''
        devuelve todos los ids de los profesores
        '''
        d = search_resource_by_type(resourcetype='Teacher')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        new_temp = []
        for i,j in temp:
            l0 = j.split()
            if len(l0)>2:
                variable = ' '.join(l0[:-2])
                l0 = f'{l0[-2]} {l0[-1]}, {variable}'
            else:
                l0 = f'{l0[1]}, {l0[0]}'
            new_temp.append((i,l0))
        temp = new_temp
        temp.sort(key = lambda x:x[1])
        
        self.set_prof_id =  [i[0] for i in temp]
        return self.set_prof_id
    
    def all_prof_text(self):
        '''
        devuelve todos los nombres de los profesores
        '''
        d = search_resource_by_type(resourcetype='Teacher')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        new_temp = []
        for i,j in temp:
            l0 = j.split()
            if len(l0)>2:
                variable = ' '.join(l0[:-2])
                l0 = f'{l0[-2]} {l0[-1]}, {variable}'
            else:
                l0 = f'{l0[1]}, {l0[0]}'
            new_temp.append((i,l0))
        temp = new_temp
        temp.sort(key = lambda x:x[1])
        self.set_prof_text =  [i[1] for i in temp]
        return self.set_prof_text
        
    def all_aula_id(self):
        '''
        devuelve todos los ids de las aulas
        '''
        d = search_resource_by_type(resourcetype='Room')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        temp.sort(key = lambda x:x[1])
        self.set_aula_id =  [i[0] for i in temp]
        return self.set_aula_id
    
    def asig_ident(self,ident):
        d = search_resource_by_type(resource=ident) # Solo por nombre profesor
        a = d.fetchall()
        return [reg['id'] for reg in a]

    def asig_times(self,ident):
        d = search_resource_by_type()
        q = d.fetchall()
        return []
    
    def all_aula_text(self):
        '''
        devuelve todos los nombres de las aulas
        '''
        d = search_resource_by_type(resourcetype='Room')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        temp.sort(key = lambda x:x[1])
        self.set_aula_text =  [i[1] for i in temp]
        return self.set_aula_text

    def all_asignatura_id(self):
        '''
        devuelve todos los ids de las asignaturas
        '''
        d = mostrar_todas_asignaturas()
        a = d.fetchall()
        a = [reg for reg in a if 'id' in reg and 'name' in reg]
        temp = {reg['id']:reg['name'] for reg in a}
        c = list(temp.items())
        c.sort(key = lambda x:x[1])
        self.set_asignatura_id =  [i[0] for i in c]
        return self.set_asignatura_id

    def all_asignatura_text(self):
        '''
        devuelve todos los nombres de las asignaturas
        '''
        d = mostrar_todas_asignaturas()
        a = d.fetchall()
        a = [reg for reg in a if 'id' in reg and 'name' in reg]
        temp = {reg['id']:reg['name'] for reg in a}
        c = list(temp.items())
        c.sort(key = lambda x:x[1])
        self.set_asignatura_text =  ['{0} ({1})'.format(i[1],i[0]) for i in c]
        return self.set_asignatura_text

    def all_curso_id(self):
        '''
        devuelve todos los ids de los cursos
        '''
        d = get_class() # Extraemos todos los recursos del tipo Class
        a = d.fetchall() # Los pasamos a un array de diccionarios para trabajar con ellos
        a = [reg for reg in a if 'id' in reg]
        self.set_curso_id =  [reg['id'] for reg in a
                              if  'PLACEHOLDER' not in reg.get('id','PLACEHOLDER')]
        return self.set_curso_id
        
    def all_curso_text(self):
        '''
        devuelve todos los nombres de los cursos
        '''
        d = get_class() # Extraemos todos los recursos del tipo Class
        a = d.fetchall() # Los pasamos a un array de diccionarios para trabajar con ellos
        a = [reg for reg in a if 'name' in reg]
        
        self.set_curso_text =  [reg['name'] for reg in a
                                if 'PLACEHOLDER' not in reg.get('name','PLACEHOLDER')]
        return self.set_curso_text
        
    def colisiones(self):
        '''
        devuelve todas las colisiones
        '''
        d = show_conflicts()
        a = d.fetchall()
        a = [reg for reg in a if  'time' in reg and 'name' in reg and 'PLACEHOLDER' not in reg['name']][0:10]
        
        
        return ['Revisar {name} la clase {Evento A} y la clase {Evento B} en {time}'.format(**reg)
                for reg in a]

    def contain2(self, datos):
        '''
        devuelve listas de registros que contengan alguno de los elementos en datos.
        El formato tiene que ser (AULA_ID,ASIG_ID,TIME_ID,NOMBRE_ASIG,NOMBRE_AULA)
        '''
        #TODO: Ahora lo que hago es coger todas las asignaturas, coger el identificador y adelante.
        # Esto puede dar problemas
        result = []
        d = search_resource_by_type()
        all_events = d.fetchall()
        for reg in all_events:
            if any(dato in reg.values() for dato in datos):
                aula_id = 'AULA_8'
                asig_id = reg['id']
                for temp in select_all_by_subject(asig_id).fetchall():
                    if 'LABORATORIO' in temp['resource'] or 'AULA' in temp['resource']:
                        aula_id = temp['resource']
                time_id = reg['time']
                nombre_asig = reg['name']
                nombre_aula = aula_id.replace('_', ' ')
                result.append((aula_id,asig_id, time_id, nombre_asig, nombre_aula))
        return list(dedupe(result))
    def contain(self, datos):
        '''
        devuelve listas de registros que contengan alguno de los elementos en datos.
        El formato tiene que ser (AULA_ID,ASIG_ID,TIME_ID,NOMBRE_ASIG,NOMBRE_AULA)
        '''
        #TODO: Ahora lo que hago es coger todas las asignaturas, coger el identificador y adelante.
        # Esto puede dar problemas
        cursor_d = mostrar_todas_asignaturas()
        nombres = cursor_d.fetchall()
        nombres = {d['id']: d['name'] for d in nombres}
        result = []
        for dato in datos:
            codigo = dato.split('_')[0]
            nombre_asig = nombres[codigo]
            for d in mostrar_todo_asignatura(codigo).fetchall():
                result.append((d['name'].replace(' ', '_'),
                               d['id'],
                               d['time'],
                               nombre_asig,
                               d['name']
                ))
        return list(dedupe(result))
    def delete(self, datos):
        '''
        borra listas de registros que contengan alguno de los elementos en datos.
        '''
        pass
    def recursos_asig(self, ident):
        '''
        Devuelve la lista de recursos de una asignatura
        '''
        d = select_all_by_subject(ident)
        a = d.fetchall()
        return list(dedupe([reg['resource'] for reg in a]))

    def aula_libre(self, tiempo):
        d = (mostrar_espacios_libres(tiempo))
        a = d.fetchall()
        return [i['id'] for i in a]
    def nombre_asig(self, ident):
        d = select_all_by_subject(ident)
        a = [reg for reg in d.fetchall() if 'id' in reg]
        for reg in a:
            if reg['id']==ident:
                return reg['name']
        else:
            return 'PLACEHOLDER'
    def cambia_dia(self, codigo, dia):
        update_events_cuatrimestre2_1718(codigo,codigo,codigo.split("_")[0],dia)
    def cambia_aula(self,codigo, aula):
        
        old_room='AULA_1'
        for text in self.recursos_asig(codigo):
            if text[0:5] in ('AULA_', 'LABOR', 'SEMIN'):
                old_room = text
        update_events_need_resources_c2_1718(codigo,old_room,aula)
    

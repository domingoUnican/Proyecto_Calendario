from helpers import dedupe
from ParserSQL_233 import *

class con_bd:

    def all_prof_id(self):
        '''
        devuelve todos los ids de los profesores
        '''
        d = search_resource_by_type(resourcetype='Teacher')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        temp.sort(key = lambda x:x[1])
        return [i[0] for i in temp]
    
    def all_prof_text(self):
        '''
        devuelve todos los nombres de los profesores
        '''
        d = search_resource_by_type(resourcetype='Teacher')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        temp.sort(key = lambda x:x[1])
        return [i[1] for i in temp]
        
    def all_aula_id(self):
        '''
        devuelve todos los ids de las aulas
        '''        
        d = search_resource_by_type(resourcetype='Room')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        temp.sort(key = lambda x:x[1])
        return [i[0] for i in temp]
    def asig_prof(self,ident):
        d = search_resource_by_type(resource=ident) # Solo por nombre profesor
        a = d.fetchall()
        return [reg['id'] for reg in a]
    def all_aula_text(self):
        '''
        devuelve todos los nombres de las aulas
        '''        
        d = search_resource_by_type(resourcetype='Room')
        a = d.fetchall()
        temp = list(dedupe( [(reg['resource'],reg['D.name']) for reg in a]))
        temp.sort(key = lambda x:x[1])
        return [i[1] for i in temp]

    def all_asignatura_id(self):
        '''
        devuelve todos los ids de las asignaturas
        '''        
        d = search_resource_by_type()
        a = d.fetchall()
        a = [reg for reg in a if 'id' in reg and 'name' in reg]
        temp = {reg['id']:reg['name'] for reg in a}
        c = list(temp.items())
        c.sort(key = lambda x:x[1])
        return [i[0] for i in c]

    def all_asignatura_text(self):
        '''
        devuelve todos los nombres de las asignaturas
        '''        
        d = search_resource_by_type()
        a = d.fetchall()
        a = [reg for reg in a if 'id' in reg and 'name' in reg]
        temp = {reg['id']:reg['name'] for reg in a}
        c = list(temp.items())
        c.sort(key = lambda x:x[1])
        return [i[1] for i in c]

    def all_curso_id(self):
        '''
        devuelve todos los ids de los cursos
        '''
        d = get_class() # Extraemos todos los recursos del tipo Class
        a = d.fetchall() # Los pasamos a un array de diccionarios para trabajar con ellos
        a = [reg for reg in a if 'id' in reg]
        return [reg['id'] for reg in a if  'PLACEHOLDER' not in reg.get('id','PLACEHOLDER')]
        
        
    def all_curso_text(self):
        '''
        devuelve todos los nombres de los cursos
        '''
        d = get_class() # Extraemos todos los recursos del tipo Class
        a = d.fetchall() # Los pasamos a un array de diccionarios para trabajar con ellos
        a = [reg for reg in a if 'name' in reg]
        
        return [reg['name'] for reg in a if 'PLACEHOLDER' not in reg.get('name','PLACEHOLDER')]
        
    def colisiones(self):
        '''
        devuelve todas las colisiones
        '''
        d = show_conflicts()
        a = d.fetchall()
        a = [reg for reg in a if  'time' in reg and 'name' in reg and 'PLACEHOLDER' not in reg['name']][0:10]
        
        
        return ['Hay un conflicto con {name} en la siguiente hora: {time}'.format(**reg) for reg in a]

    def contain(self, datos):
        '''
        devuelve listas de registros que contengan alguno de los elementos en datos.
        El formato tiene que ser (AULA_ID,ASIG_ID,TIME_ID,NOMBRE_ASIG,NOMBRE_AULA)
        '''
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
            if 'AULA' in text or 'LAB' in text:
                old_room = text
        update_events_need_resources_c2_1718(codigo,old_room,aula)
    

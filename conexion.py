from helpers import dedupe
from ParserSQL_233 import *

class con_bd:

    def all_prof_id(self):
        '''
        devuelve todos los ids de los profesores
        '''
        d = search_resource_by_type(resourcetype='Teacher')
        a = d.fetchall()
        return [reg['resource'] for reg in a]
    def all_prof_text(self):
        '''
        devuelve todos los nombres de los profesores
        '''
        d = search_resource_by_type(resourcetype='Teacher')
        a = d.fetchall()
        return [reg['D.name'] for reg in a]
        
    def all_aula_id(self):
        '''
        devuelve todos los ids de las aulas
        '''        
        d = search_resource_by_type(resourcetype='Room')
        a = d.fetchall()
        a = [reg for reg in a if 'resource' in reg ]
        return [reg['resource'] for reg in a]

    def all_aula_text(self):
        '''
        devuelve todos los nombres de las aulas
        '''        
        d = search_resource_by_type(resourcetype='Room')
        a = d.fetchall()
        a = [reg for reg in a if 'D.name' in reg ]
        return [reg['D.name'] for reg in a]

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
        a = [reg for reg in a if 'name' in a]
        return [reg['name'] for reg in a if 'PLACEHOLDER' not in reg.get('name','PLACEHOLDER')]
        
    def colisiones(self):
        '''
        devuelve todas las colisiones
        '''
        return ["Una colision"]
        d = show_conflicts()
        a = d.fetchall()
        a = [reg for reg in a if  'time' in reg and 'name' in reg]
        for reg in a:
            print(a)
        return ['Hay un conflicto con {name} en la siguiente hora {time}'.format(reg) for reg in a]

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
                asig_id = reg['id']
                for temp in select_all_by_subject(asig_id).fetchall():
                    if 'LABORATORIO' in temp['resource'] or 'AULA' in temp['resource']:
                        aula_id = temp['resource']
                time_id = reg['time']
                nombre_asig = reg['name']
                nombre_aula = aula_id.replace('_', ' ')
                result.append((aula_id,asig_id, time_id, nombre_asig, nombre_aula))
        
    def delete(self, datos):
        '''
        borra listas de registros que contengan alguno de los elementos en datos.
        '''
        pass
    
    

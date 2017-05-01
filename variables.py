# -*- coding: utf-8 -*-

import os

DIR = os.path.join('.','datos')
PROFS = [os.path.join(DIR, 'profesores_doblegrado.csv'),
         os.path.join(DIR, 'profesores_fisica.csv'),
         os.path.join(DIR, 'profesores_matematicas.csv'),
         os.path.join(DIR, 'profesores_informática.csv'),
]
ASIG = [os.path.join(DIR, 'asignaturas.csv'),
]
ROOMS = os.path.join(DIR, 'aulas.csv')
OUT = os.path.join(DIR, 'outfile_nuevo.xml')
GRUPOS = ['AntesDescanso', 'DespuesDescanso','Tarde']
GRUPOS_NAMES = {
    'AntesDescanso': 'Antes del descanso',
    'DespuesDescanso':'Despues del descanso',
    'Tarde':'Tarde'
}
DIAS = ['Lunes', 'Martes', 'Miercoles', 'Jueves', 'Viernes']
HORAS = [str(i) for i in range(1,10)]
RECURSOS = ['Teacher', 'Room', 'Class','Laboratory']
RECURSOS_NAME = {
    'Teacher': 'Profesores',
    'Room': 'Aulas',
    'Class': 'Cursos',
    'Laboratory':'Laboratorio'
}
DEGREES = ['DOBLE','FISICA','MATEMATICAS','INFORMATICA']
TYPES_ROOMS = {'Aula_Grande':'Room',
               'Aula_Normal':'Room',
               'Laboratorio':'Laboratory',
               'Aula_Pequena':'Room'}
TYPES_ROOMS_NAMES = {
    'Aula_Grande':'Aula Grande',
    'Aula_Normal':'Aula Normal',
    'Laboratorio':'Laboratorio',
    'Aula_Pequena':'Aula Pequena'
}
CLASSES_NAME = dict()
for degree in DEGREES:
    for i in range(1,5):
        key = 'Class_%s1_%s'%(str(i),degree )
        value = 'Clase %s (grupo 1) de %s'%(str(i),degree)
        CLASSES_NAME[key] = value
        if i<=3:
            key = 'Class_%s2_%s'%(str(i),degree )
            value = 'Clase %s (grupo 2) de %s'%(str(i),degree)
            CLASSES_NAME[key] = value
CLASSES = list(CLASSES_NAME.keys())
CLASSES.sort()
COURSE = dict(zip(PROFS,DEGREES))
for asig in ASIG:
    for degree in DEGREES:
        if degree.lower() in asig:
            COURSE[asig] = degree

N_GROUPS = {str(i):1 for i in range(1,5)}

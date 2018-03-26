#Inserción de una ASIGNATURA (no de cada una de las clases que se imartan de la misma) - ENTRADA=(código único de la asignatura G... , nombre o etiqueta identificativa, cuatrimestee en que se imparte):
INSERT INTO eventgroups (id,name,cuatrimestre) VALUES (CODIGO_ASIGNATURA_AQUI,NOMBRE_AQUI,CUATRIMESTRE_AQUI_1_o_2);

#Inserción de una CLASE ESPECIFICA DE UNA ASIGNATURA, con ENTRADA=(código de la clase específica, nombre o etiqueta de la clase específica, código de la asignatura a la que corresponde(eventgroups.id), ranura de tiempo que se le asigna (times.id))
INSERT INTO events_cuatrimestre2_1718 (id, name, eventgroup, time) VALUES (CODIGO_EVENTO_CLASE, ETIQUETA_EVENTO_CLASE, CODIGO_ASIGNATURA, TIEMPO);

#Asignacion de AULA a una clase concreta de una asignatura. ENTRADA=(código de la clase específica (events_cuatrimestre2_1718.id), código del recurso aula (resources.id))
INSERT INTO events_need_resources_c2_1718 (event, resource) VALUES (CODIGO_EVENTO_CLASE, CODIGO_RECURSO_AULA);

#Asignacion de GRUPO DE ALUMNOS (class) a una clase concreta de una asignatura. ENTRADA=(código de la clase específica (events_cuatrimestre2_1718.id), código del recurso grupo de alumnos/class (resources.id))
INSERT INTO events_need_resources_c2_1718 (event, resource) VALUES (CODIGO_EVENTO_CLASE, CODIGO_RECURSO_GRUPO);

#Asignacion de PROFESOR a una clase concreta de una asignatura. ENTRADA=(código de la clase específica (events_cuatrimestre2_1718.id), código del recurso profesor (resources.id))
INSERT INTO events_need_resources_c2_1718 (event, resource) VALUES (CODIGO_EVENTO_CLASE, CODIGO_RECURSO_PROFESOR);
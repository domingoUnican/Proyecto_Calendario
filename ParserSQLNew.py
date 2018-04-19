import pymysql

# Cambiar user y password asi como db a las del servidor local
connection = pymysql.connect(host='localhost',user='root',password='6f8AFF203',db='scheduledb',charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor, autocommit=True)

cursor = connection.cursor()

# ---------------------------------------------------------------------------------------------
# Se le pasa como argumento uno de los diccionario y retorna un string donde muestra los valores
# ---------------------------------------------------------------------------------------------
def dict_toString(dict):
    for row in dict:
        print(row)

# ---------------------------------------------------------------------------------------------
# Transforma un cursor en un array de diccionarios
# ---------------------------------------------------------------------------------------------
def dict_toArray(dict):
    array = dict.fetchall()
    return array

# ---------------------------------------------------------------------------------------------
# Mostrar todos los profesores
# Ejemplo: dict_toString(mostrar_todos_profesores())
# ---------------------------------------------------------------------------------------------
def mostrar_todos_profesores():
    query = f'SELECT DISTINCT resources.id, resources.name, resources.resourcetype FROM resources WHERE resources.resourcetype="Teacher";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Mostrar todas la clases
# Ejemplo: dict_toString(mostrar_todas_clases())
# ---------------------------------------------------------------------------------------------
def mostrar_todas_clases():
    query = f'SELECT DISTINCT resources.id, resources.name, resources.resourcetype FROM resources WHERE resources.resourcetype="Class";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Mostar todas las aulas
# Ejemplo: dict_toString(mostrar_todas_aulas())
# ---------------------------------------------------------------------------------------------
def mostrar_todas_aulas():
    query = f'SELECT DISTINCT resources.id, resources.name, resources.resourcetype FROM resources WHERE resources.resourcetype="Room";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Mostrar todas las asignaturas
# Ejemplo: dict_toString(mostrar_todas_asignaturas())
# ---------------------------------------------------------------------------------------------
def mostrar_todas_asignaturas():
    query = f'SELECT eventgroups.id, eventgroups.name, eventgroups.cuatrimestre FROM eventgroups;'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Dado un codigo de asignatura muestra todas sus horas de teoria y laboratorio
# mostrando los atributos (asignatura, id, nombre y tiempo)
# Ejemplo: dict_toString(mostrar_horarios_asignaturas('G100'))
# ---------------------------------------------------------------------------------------------
def mostrar_horarios_asignaturas(cod_asignatura):
    query = f'SELECT events_cuatrimestre2_1718.eventgroup, events_cuatrimestre2_1718.id, events_cuatrimestre2_1718.name, events_cuatrimestre2_1718.time ' \
            f'FROM  events_cuatrimestre2_1718 ' \
            f'WHERE events_cuatrimestre2_1718.eventgroup="{cod_asignatura}";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Dado un codigo de asignatura (Ej: G100) retorna un array con todos las clases teoricas y
# laboratorios mostrando los atributos (id asignatura, id clase, tiempo, lugar y profesor)
# Ejemplo: dict_toString(mostrar_todo_asignatura('G100'))
# ---------------------------------------------------------------------------------------------
def mostrar_todo_asignatura(cod_asignatura):
    query = f'SELECT events_cuatrimestre2_1718.eventgroup, events_cuatrimestre2_1718.id, events_cuatrimestre2_1718.time, ' \
            f'res_aulas.name, res_prof.name FROM  events_cuatrimestre2_1718' \
            f'	INNER JOIN events_need_resources_c2_1718 enr_aulas ON enr_aulas.event= events_cuatrimestre2_1718.id' \
            f'    INNER JOIN resources res_aulas ON enr_aulas.resource=res_aulas.id' \
            f'    INNER JOIN events_need_resources_c2_1718 enr_prof ON enr_prof.event= events_cuatrimestre2_1718.id' \
            f'    INNER JOIN resources res_prof ON enr_prof.resource=res_prof.id' \
            f'    WHERE events_cuatrimestre2_1718.eventgroup="{cod_asignatura}" ' \
            f'		AND res_prof.resourcetype="Teacher"' \
            f'AND res_aulas.resourcetype="Room";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Dado un tiempo concreto (Ej: Lunes1) retorna un array con todos
# los recursos libre en ese momento mostrando sus atributos (id, nombre, tipo recurso, color)
# Ejemplo: dict_toString(mostrar_espacios_libres('Lunes1'))
# ---------------------------------------------------------------------------------------------
def mostrar_espacios_libres(tiempo):
    query = f'SELECT DISTINCT res.id, res.name, res.resourcetype, res.color FROM events_cuatrimestre2_1718' \
            f'	INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event' \
            f'    INNER JOIN resources res ON enr.resource=res.id' \
            f'    WHERE res.id NOT IN (' \
            f'    SELECT DISTINCT res.id ' \
            f'					FROM events_cuatrimestre2_1718' \
            f'					INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event' \
            f'					INNER JOIN resources res ON enr.resource=res.id' \
            f'					WHERE events_cuatrimestre2_1718.time="{tiempo}")' \
            f'                  AND res.resourcetype="Room";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Dado un tiempo concreto (Ej: Lunes1) y un tipo de espacio requerido (Ej: gr_Aula_Grande)
# retorna un array con todos los recursos libre en ese momento mostrando sus atributos
# (id, nombre, tipo recurso, color y tipo de espacio)
# Ejemplo: dict_toString(mostrar_espacios_libres_por_tipo('Lunes1','gr_Aula_Grande'))
# ---------------------------------------------------------------------------------------------
def mostrar_espacios_libres_por_tipo(tiempo,tipo_espacio):
    query = f'SELECT DISTINCT res.id, res.name, res.resourcetype, res.color, rg.id FROM events_cuatrimestre2_1718' \
            f'	INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event' \
            f'    INNER JOIN resources res ON enr.resource=res.id' \
            f'    INNER JOIN resources_to_resourcegroups rtr ON res.id=rtr.resource' \
            f'    INNER JOIN resourcegroups rg ON rtr.resourcegroup=rg.id ' \
            f'    WHERE res.id NOT IN (' \
            f'    SELECT DISTINCT res.id ' \
            f'					FROM events_cuatrimestre2_1718' \
            f'					INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event' \
            f'					INNER JOIN resources res ON enr.resource=res.id' \
            f'					WHERE events_cuatrimestre2_1718.time="{tiempo}")' \
            f'                  AND res.resourcetype="Room" AND rg.id="{tipo_espacio}";'
    cursor.execute(query)
    return cursor

# ---------------------------------------------------------------------------------------------
# Metodo que permite insertar una nueva asignatura
#
# ---------------------------------------------------------------------------------------------
def insert_eventgroup(id, name, curso, cuatrimestre):
    query = f'INSERT INTO eventgroups(id, name, curso, cuatrimestre, h_teoria, h_practica) ' \
            f'VALUES ("{id}","{name}","{curso}","{cuatrimestre},NULL,NULL);'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Metodo que permite insertar un nuevo recurso (profesor, aula , ...) asignado a una asignatura
# ---------------------------------------------------------------------------------------------
def insert_events_need_resources_c2_1718(event,resource):
    query = f'INSERT INTO events_need_resources_c2_1718(event, resource)' \
            f'VALUES ("{event}","{resource}");'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Metodo que permite insertar una nueva sesion (teoria, laboratorio, ...) a una asignatura ya
# existente en un horario concreto
# ---------------------------------------------------------------------------------------------
def insert_events_cuatrimestre2_1718(event,name,eventgroup,time):
    query = f'INSERT INTO events_cuatrimestre2_1718(id, name, eventgroup, time)' \
            f'VALUES ("{event}","{name}","{eventgroup}","{time}");'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Metodo que permite insertar un nuevo recurso (profesor, aula, clase, ...)
# ---------------------------------------------------------------------------------------------
def insert_resources(resource,name,resourcetype):
    query = f'INSERT INTO resources (id,name,resourcetype,color)' \
            f'VALUES ("{resource}","{name}","{resourcetype}",NULL);'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Dada una tabla y un nombre de campo con un valor asignado, borra todas las entradas que
# encuentra coincidentes
# ---------------------------------------------------------------------------------------------
def delete_row(table,tag,value):
    query = f'DELETE FROM "{table}" WHERE "{tag}" = {value}";'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Dado un identifigador de recurso como primer argumento actualiza el valor de todas las
# entradas coincidentes con dicho identificador a los parametros que se le pasen
# ---------------------------------------------------------------------------------------------
def update_resources(resource,name,resourceType):
    query = f'UPDATE resources SET name="{name}",resourceType="{resourceType}" WHERE id="{resource}";'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Para un evento de una asignatura (teoria, laboratorio,...) actualiza los valores que se
# le pasan como argumentos al metodo
# Ejemplo: update_events_cuatrimestre2_1718('G100_pl_1_a','G100_pl_1_a','G100','Martes2')
# ---------------------------------------------------------------------------------------------
def update_events_cuatrimestre2_1718(event,name,eventgroup,time):
    query = f'UPDATE events_cuatrimestre2_1718 SET name="{name}",eventgroup="{eventgroup}",time="{time}"' \
            f'WHERE id="{event}";'
    cursor.execute(query)
    return 1
update_events_cuatrimestre2_1718('G100_pl_1_a','G100_pl_1_a','G100','Martes2')

# ---------------------------------------------------------------------------------------------
# Dado un evento y dadas una antigua y nueva ubicaciones actualiza donde tiene lugar el evento
# Ejemplo: update_events_need_resources_c2_1718('G671_t2','AULA_1','AULA_2')
# ---------------------------------------------------------------------------------------------
def update_events_need_resources_c2_1718(event,old_room,new_room):
    query = f'UPDATE events_need_resources_c2_1718 SET event="{event}",resource="{new_room}" WHERE event="{event}" ' \
        f' AND resource ="{old_room}";'
    cursor.execute(query)
    return 1

# ---------------------------------------------------------------------------------------------
# Permite insretar un nuevo tiempo
# ---------------------------------------------------------------------------------------------
def insert_time(id, name, day,timegroup):
    query = f'INSERT INTO times (id,name,day,timegroup) VALUES ("{id}","{name}","{day}","{timegroup}")'
    cursor.execute(query)
    return 1

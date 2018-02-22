import pymysql

# Cambiar user y password asi como db a las del servidor local
connection = pymysql.connect(host='localhost',user='Domingo',password='Domingo',db='scheduledb',charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor, autocommit=True)

cursor = connection.cursor()

def select_tables(table_name):
    query = "SELECT * FROM "+ table_name +";"
    cursor.execute(query)
    return cursor;

def dict_toString(dict):
    for row in dict:
        print(row)

def select_all_by_prof(professor):
    query = "SELECT event from events_need_resources_c2_1718 where resource = '"+str(professor)+"';"
    cursor.execute(query)
    return cursor

def select_all_by_room(room):
    query = "SELECT event from events_need_resources_c2_1718 where resource = '"+str(room)+"';"
    cursor.execute(query)
    return cursor

def select_all_by_subject(subject):
    query = "SELECT resource from events_need_resources_c2_1718 where event = '"+str(subject)+"';"
    cursor.execute(query)
    return cursor

def show_resourceTypes():
    query = "SELECT * from resourcetypes;"
    cursor.execute(query)
    return cursor

def show_times():
    query = "SELECT * from times"
    cursor.execute(query)
    return cursor

def show_conflicts():
    query = "SELECT resources.id, resources.name, ecA.name as 'Evento A', ecB.name as 'Evento B', ecA.time FROM resources " \
            "INNER JOIN events_need_resources_c2_1718 enrA ON resources.id=enrA.resource " \
            "INNER JOIN events_cuatrimestre2_1718 ecA ON ecA.id=enrA.event INNER JOIN events_need_resources_c2_1718 enrB " \
            "ON resources.id=enrB.resource INNER JOIN events_cuatrimestre2_1718 ecB ON enrB.event=ecB.id " \
            "WHERE ecA.id!=ecB.id AND ecA.time=ecB.time;"
    cursor.execute(query)
    return cursor

#dict_toString(show_conflicts())

def get_class():
    query = "SELECT * FROM resources WHERE resourcetype = 'Class';"
    cursor.execute(query)
    return cursor

def search_resource_by_type(resource='',resourcetype='',time=''):
    query = "SELECT A.id, A.eventgroup, A.time, B.resource, C.name, C.curso, C.cuatrimestre, D.resourcetype, D.name " \
            "FROM events_cuatrimestre2_1718 AS A " \
            "INNER JOIN events_need_resources_c2_1718 AS B ON A.id = B.event " \
            "INNER JOIN eventgroups as C ON A.eventgroup = C.id " \
            "INNER JOIN resources as D ON B.resource = D.id "

    if (resource != '') & (time != ''):
        query = query + "WHERE A.time = '" + str(time) + "' AND B.resource = '"+str(resource)+"';"
    elif resource != '':
        query = query + "WHERE B.resource = '"+str(resource)+"'; "
    else:
        if (resourcetype != '') & (time != ''):
            query = query + "WHERE A.time = '" + str(time) + "' AND D.resourcetype = '" + str(resourcetype) + "';"
        elif (time != ''):
            query = query + "WHERE A.time = '" + str(time) + "';"
        elif resourcetype != '':
            query = query + "WHERE D.resourcetype = '" + str(resourcetype) + "'; "
        elif (resource == '') & (resourcetype == '') & (time == ''):
            query = query + ";"

    cursor.execute(query)
    return cursor

def insert_eventgroup(id, name, curso, cuatrimestre):
    query = "INSERT INTO eventgroups(id, name, curso, cuatrimestre, h_teoria, h_practica) " \
            "VALUES ('"+str(id)+"','"+str(name)+"','"+str(curso)+"','"+str(cuatrimestre)+"',NULL,NULL);"
    cursor.execute(query)
    return 1

def insert_events_need_resources_c2_1718(event,resource):
    query = "INSERT INTO events_need_resources_c2_1718(event, resource)" \
            "VALUES ('"+str(event)+"','"+str(resource)+"');"
    cursor.execute(query)
    return 1

def insert_events_cuatrimestre2_1718(event,name,eventgroup,time):
    query = "INSERT INTO events_cuatrimestre2_1718(id, name, eventgroup, time)" \
            "VALUES ('"+str(event)+"','"+str(name)+"','"+str(eventgroup)+"','"+str(time)+"');"
    cursor.execute(query)
    return 1

def insert_resources(resource,name,resourcetype):
    query = "INSERT INTO resources (id,name,resourcetype,color)" \
            "VALUES ('"+str(resource)+"','"+str(name)+"','"+str(resourcetype)+"',NULL);"
    cursor.execute(query)
    return 1

def delete_row(table,tag,value):
    query = "DELETE FROM "+str(table)+" WHERE "+str(tag)+" = '"+str(value)+"';"
    cursor.execute(query)
    return 1

def update_resources(resource,name,resourceType):
    query = "UPDATE resources SET name='"+str(name)+"',resourceType='"+str(resourceType)+"' WHERE id='"+str(resource)+"';"
    print(query)
    cursor.execute(query)
    return 1

#update_resources('A','BBBB','Class')

def update_events_cuatrimestre2_1718(event,name,eventgroup,time):
    query = "UPDATE events_cuatrimestre2_1718 SET name='"+str(name)+"',eventgroup='"+str(eventgroup)+"',time='"+str(time)+"'" \
            "WHERE id='"+str(event)+"';"
    cursor.execute(query)
    return 1

#update_events_cuatrimestre2_1718('AA','TEoria','G100','Viernes3')


def update_events_need_resources_c2_1718(event,old_room,new_room):
    query = "UPDATE events_need_resources_c2_1718 SET event='"+str(event)+"',resource='"+str(new_room)+"' WHERE event='"+str(event)+"' " \
            " AND resource ='"+str(old_room)+"';"
    cursor.execute(query)
    return 1

def insert_time(id, name, day,timegroup):
    query = "INSERT INTO times (id,name,day,timegroup) VALUES ('"+str(id)+"','"+str(name)+"','"+str(day)+"','"+str(timegroup)+"')"
    cursor.execute(query)
    return 1

#dict_toString(select_all_by_prof('adolfo_garandal_martin'))
print("---")
#dict_toString(select_all_by_room("LABORATORIO_4"))
print("---")
#dict_toString(select_all_by_subject("G686_pl_1"))
print("---")
#dict_toString(show_conflicts('Teacher'))
print("---")


### EJEMPLOS ###

### Como actualizar el time de un registro ###

# d = search_resource_by_type(resource='diego_herranz_munoz',time='Viernes2') # Buscamos por el recurso (profesor, aula o clase)
# a = d.fetchall() # Extraemos todos los datos en un array de diccionarios
# print(a)
# # Si la busqueda no encuentra ningun resgistro retornara un array vacio

# update_events_cuatrimestre2_1718(a[0]['id'],a[0]['id'],a[0]['eventgroup'],'Viernes4') # Actualizamos los datos necesarios (Viernes4)
# d = search_resource_by_type(resource='diego_herranz_munoz',time='Viernes4') # Buscamos de nuevo para ver que se realizan los cambios
# a = d.fetchall()
# print(a)

# ### Como actualizar el room de un registro ###

# old_room = 'AULA_1'
# new_room = 'AULA_2'
# event = 'G671_t2'
# update_events_need_resources_c2_1718(event,old_room,new_room)
# # El metodo busca el registro que coincide con el evento y la room actual y lo actualiza a la nueva room
# ### Como extraer todas las clases disponibles ###


# d = get_class() # Extraemos todos los recursos del tipo Class
# a = d.fetchall() # Los pasamos a un array de diccionarios para trabajar con ellos

# ### Como ver los registros actuales filtrando por un recurso ###

# d = search_resource_by_type(resource='diego_herranz_munoz',time='Viernes2') # Por nombre profesor y hora
# a = d.fetchall()

# d = search_resource_by_type(resource='diego_herranz_munoz') # Solo por nombre profesor
# a = d.fetchall()

# d = search_resource_by_type(resource='AULA_1',time='Viernes2') # Por room y hora
# a = d.fetchall()

# d = search_resource_by_type(resource='AULA_1') # Solo por room
# a = d.fetchall()

# d = search_resource_by_type(resourcetype='Teacher') # Por tipo de recurso (Teacher, Class, Room)
# a = d.fetchall()

# d = search_resource_by_type(resourcetype='Class',time='Viernes2') # Por tipo de recurso y hora
# a = d.fetchall()

# d = search_resource_by_type() # Mostrartodo sin filtro
# a = d.fetchall()

# ### Para insertar datos nuevos en las tablas simplemente utilizar los insert con los datos que se quieran introducir ###

# ### Para borrar un registro de una tabla ###

# delete_row('resources','id','A') # Borrar de la tabla resources los registros cuyo id sea igual a 'A'
# # Esto puede hacerse para cualquier tabla campo y valor


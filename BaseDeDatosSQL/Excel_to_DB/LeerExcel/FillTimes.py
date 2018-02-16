# -*- coding: utf-8 -*-
import mysql.connector

#fijar la constante de nombre
DB_NAME='scheduleDB'
#conectarse a la BD
cnx = mysql.connector.connect(user='root', password='itsasecret',
                              host='localhost',
                              database=DB_NAME)


#asegurar que estamos conectados a la BD correcta
cnx.database=DB_NAME

#obtener el cursor para operar con la BD
cursor=cnx.cursor()

days = ['Lunes','Martes','Miercoles','Jueves','Viernes']

#definimos las queries (incluir la palabra IGNORE asegura que no se produzcan errores si hay datos repetidos. Así simplemente lo saltaría sin introducirlo)
#el sistema es bastante especial para ejecutar las queries;el mejor modo que he encontrado es definirlas aquí y sustituir  las variables %s por argumentos posteriormente
query=( "INSERT IGNORE INTO times" 
        "(id, name, day, timegroup)" 
        "VALUES (%s, %s, %s, %s)")

#se generar todas las combinaciones de dia y hora
for i in range(5):
    values=(days[i]+'1',days[i]+' - 1',days[i],'AntesDescanso')
    cursor.execute(query,values)
    values=(days[i]+'2',days[i]+' - 2',days[i],'AntesDescanso')
    cursor.execute(query,values)
    values=(days[i]+'3',days[i]+' - 3',days[i],'DespuesDescanso')
    cursor.execute(query,values)
    values=(days[i]+'4',days[i]+' - 4',days[i],'DespuesDescanso')
    cursor.execute(query,values)
    values=(days[i]+'5',days[i]+' - 5',days[i],'DespuesDescanso')
    cursor.execute(query,values)
    values=(days[i]+'6',days[i]+' - 6',days[i],'Tarde')
    cursor.execute(query,values)
    values=(days[i]+'7',days[i]+' - 7',days[i],'Tarde')
    cursor.execute(query,values)
    values=(days[i]+'8',days[i]+' - 8',days[i],'Tarde')
    cursor.execute(query,values)
    values=(days[i]+'9',days[i]+' - 9',days[i],'Tarde')
    cursor.execute(query,values)
    
    #Esta es realmente la 6 y ultima hora de la manhana, pero se le asigno el numero 10 porque fue asignada posteriormente 
    #(no estaba en la version previa de la base de conocimiento y se anhadio al trabajar con los horarios nuevos)
    values=(days[i]+'10',days[i]+' - 10',days[i],'DespuesDescanso')
    cursor.execute(query,values)


#aprobar los cambios
cnx.commit()
#cerrar cursor y conector
cursor.close()                 
cnx.close()
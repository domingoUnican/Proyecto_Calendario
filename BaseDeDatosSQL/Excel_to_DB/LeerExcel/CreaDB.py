#!/usr/bin/env python
# -*- coding: utf-8 -*-
import mysql.connector

#conectarse a la BD
cnx = mysql.connector.connect(user='root', password='itsasecret',
                              host='127.0.0.1',
                              database='scheduleDB')


#fijar la constante de nombre y asegurar que estamos conectados a la BD correcta
DB_NAME='scheduleDB'
cnx.database=DB_NAME

#obtener el cursor para operar con la BD
cursor=cnx.cursor()

#leer el archivo con la query de creacion de tablas
fd = open('CreaBD.sql', 'r')
sqlFile = fd.read()
fd.close()
#ejecutar la consulta
cursor.execute(sqlFile)

#para completar
##cnx.commit()

#cierre de la conexion
cursor.close()
cnx.close()
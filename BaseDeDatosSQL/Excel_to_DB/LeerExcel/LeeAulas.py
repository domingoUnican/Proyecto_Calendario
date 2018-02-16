# -*- coding: utf-8 -*-
from openpyxl import load_workbook
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

######NO SE USA, SE REALIZÓ SU FUNCIÓN DIRECTAMENTE MEDIANTE QUERY

#cerrar cursor y conector
cursor.close()
cnx.close()
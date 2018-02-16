# -*- coding: utf-8 -*-
import openpyxl
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

#definimos las queries (incluir la palabra IGNORE asegura que no se produzcan errores si hay datos repetidos. Así simplemente lo saltaría sin introducirlo)
#el sistema es bastante especial para ejecutar las queries;el mejor modo que he encontrado es definirlas aquí y sustituir  las variables %s por argumentos posteriormente
query=( "INSERT IGNORE INTO eventgroups" 
        "(id, name, curso, cuatrimestre)" 
        "VALUES (%s, %s, %s, %s)")


#acceso al primer documento de excel
wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_grado.xlsx')

#acceso a cada una de las páginas que interesan
sheet_mat=wb.get_sheet_by_name("G-MATEMATICA")
sheet_inf=wb.get_sheet_by_name("G-INFORMATIC")
sheet_fis=wb.get_sheet_by_name("G-FISICA")

#combinamos los datos semejantes para agilizar el proceso de iteración sobre ellos
cells_profesores=sheet_mat['A6':'F105']+sheet_inf['A6':'F133']+sheet_fis['A6':'F166']

#segundo documentp
wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_doble_grado.xlsx')

#incorporamos también el otro documento
sheet_doble=wb.get_sheet_by_name("DT-FISIMATE")
cells_profesores=cells_profesores+sheet_doble['A6':'F98']


#una vez tenemos todos los datos podemos introducirlos en la base de datos
profesores=[]
for row in cells_profesores:
    codigo=row[0].value.replace("(*)","")
    name= row[1].value   
    curso=row[3].value    
    cuatrimestre=row[5].value
    cursor.execute(query,(codigo,name,curso,cuatrimestre))
    #print(profesor+ " "+ prof_corregido)

#aprobar todos los cambios, cerrar cursor y conector
cnx.commit()
cursor.close()
cnx.close()
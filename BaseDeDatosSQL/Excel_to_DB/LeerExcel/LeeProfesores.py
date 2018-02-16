# -*- coding: utf-8 -*-
import openpyxl
import mysql.connector

#SE EJECUTA LA OTRA (LeeProfesores_20172018); ESTA QUEDA OBSOLETA

#conectarse a la BD
cnx = mysql.connector.connect(user='root', password='itsasecret',
                              host='127.0.0.1',
                              database='scheduleDB')


#fijar la constante de nombre y asegurar que estamos conectados a la BD correcta
DB_NAME='scheduleDB'
cnx.database=DB_NAME

#obtener el cursor para operar con la BD
cursor=cnx.cursor()
#definimos la queri
query1=( "INSERT IGNORE INTO resources" 
        "(id, name, resourcetype)" 
        "VALUES (%s, %s, %s)")

query2=("INSERT IGNORE INTO resources_to_resourcegroups"
        "(resource,resourcegroup)" 
        "VALUES (%s, %s)")

#acceso al primer documento de excel
wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_grado.xlsx')

#acceso a cada una de las páginas que interesan
sheet_mat=wb.get_sheet_by_name("G-MATEMATICA")
sheet_inf=wb.get_sheet_by_name("G-INFORMATIC")
sheet_fis=wb.get_sheet_by_name("G-FISICA")

#combinamos los datos semejantes para agilizar el proceso de iteración sobre ellos
cells_profesores=sheet_mat['G6':'G105']+sheet_inf['G6':'G133']+sheet_fis['G6':'G166']

#segundo documentp
wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_doble_grado.xlsx')
#se accede a la pagina y se incorporan las celdas requeridas
sheet_doble=wb.get_sheet_by_name("DT-FISIMATE")
cells_profesores=cells_profesores+sheet_doble['G6':'G98']


#una vez tenemos todos los datos podemos introducirlos en la base de datos
profesores=[]
for row in cells_profesores:
    for cell in row:
        profesor= cell.value
        prof_corregido=profesor.replace(" ","_").replace(u"\u00F1","n").replace(u"\u00D1","n").lower()
        
        cursor.execute(query1,(prof_corregido,profesor,'Teacher'))
        cursor.execute(query2,(prof_corregido, 'gr_Teacher'))
    #print(profesor+ " "+ prof_corregido)


cnx.commit()
cursor.close()
cnx.close()
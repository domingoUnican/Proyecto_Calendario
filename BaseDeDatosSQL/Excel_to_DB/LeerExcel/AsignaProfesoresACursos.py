# -*- coding: utf-8 -*-
import openpyxl
import mysql.connector


## NO SE UTILIZA
## SIRVIO PARA ELABORAR EL CODIGO PERO LOS PROFESORES SE ANADEN AL MISMO TIEMPO QUE LAS ASIGNATURAS EN EL OTRO SCRIPT (LeeHorarioTratado)

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
query=( "INSERT IGNORE INTO events_need_resources" 
        "(event, resource)" 
        "VALUES (%s, %s)")


#acceso al primer documento de excel
wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_grado.xlsx')

#acceso a cada una de las páginas que interesan
sheet_mat=wb.get_sheet_by_name("G-MATEMATICA")
sheet_inf=wb.get_sheet_by_name("G-INFORMATIC")
sheet_fis=wb.get_sheet_by_name("G-FISICA")

#combinamos los datos semejantes para agilizar el proceso de iteración sobre ellos
cells_profesores=sheet_mat['A6':'G111']+sheet_inf['A6':'G130']+sheet_fis['A6':'G154']


#segundo documentp
wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_doble_grado.xlsx')

sheet_doble=wb.get_sheet_by_name("DT-FISIMATE")
cells_profesores=cells_profesores+sheet_doble['A6':'G98']

#una vez tenemos todos los datos podemos introducirlos en la base de datos
profesores=[]
for row in cells_profesores:
    codigo=row[0].value.replace("(*)","")
    profesor= row[6].value.replace(" ","_").replace(u"\u00F1","n").replace(u"\u00D1","n").lower()
    cursor.execute(query,(codigo,profesor))
    #print(codigo+ " "+ profesor)
    

#aprobar todos los cambios, cerrar cursor y conector
cnx.commit()
cursor.close()
cnx.close()
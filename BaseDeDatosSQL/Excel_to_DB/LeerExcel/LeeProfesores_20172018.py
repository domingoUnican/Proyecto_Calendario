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
query1=( "INSERT IGNORE INTO resources" 
        "(id, name, resourcetype)" 
        "VALUES (%s, %s, %s)")

query2=("INSERT IGNORE INTO resources_to_resourcegroups"
        "(resource,resourcegroup)" 
        "VALUES (%s, %s)")

#acceso al primer documento de excel
wb=openpyxl.load_workbook('C:\Users\Manuel\Desktop\Proyecto_Calendario-master\profesores_Grado.xlsx')

#acceso a cada una de las páginas que interesan
sheet_mat=wb.get_sheet_by_name("G-MATEMATICA")
sheet_inf=wb.get_sheet_by_name("G-INFORMATIC")
sheet_fis=wb.get_sheet_by_name("G-FISICA")

#combinamos los datos semejantes para agilizar el proceso de iteración sobre ellos
cells_profesores=sheet_mat['G6':'G111']+sheet_inf['G6':'G130']+sheet_fis['G6':'G154']

# #segundo documentp
# wb=openpyxl.load_workbook('C:\\Users\\Manuel\\Desktop\\Proyecto_Calendario-master\\Proyecto_Calendario-master\\xls_files\\Horas_clase_profesor_doble_grado.xlsx')
# 
# sheet_doble=wb.get_sheet_by_name("DT-FISIMATE")
# cells_profesores=cells_profesores+sheet_doble['G6':'G98']


#una vez tenemos todos los datos podemos introducirlos en la base de datos
profesores=[]
for row in cells_profesores:
    for cell in row:
        #se extraen los valores relevantes de la fila 
        profesor= cell.value
        #simplificamos el nombre, en minusculas, sin espacios ni caracteres extraños; esto servirá de clave primaria en la BD
        prof_corregido=profesor.replace(" ","_").replace(u"\u00F1","n").replace(u"\u00D1","n").lower()
        
        #se ejecutan las queries necesarias; es util predefinir todo lo posible para introducir el menor número de datos nuevos en este paso
        cursor.execute(query1,(prof_corregido,profesor,'Teacher'))
        cursor.execute(query2,(prof_corregido, 'gr_Teacher'))
    #print(profesor+ " "+ prof_corregido)

#commit para aplicar los cambios
cnx.commit()
#sierre del cursor y la conexión
cursor.close()
cnx.close()
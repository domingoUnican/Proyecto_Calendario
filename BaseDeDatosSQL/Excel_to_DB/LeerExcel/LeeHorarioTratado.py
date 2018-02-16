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
query1=( "INSERT IGNORE INTO events_cuatrimestre2_1718" 
        "(id, name, eventgroup, time)" 
        "VALUES (%s, %s, %s, %s)")

query2=("INSERT IGNORE INTO events_need_resources_c2_1718"
        "(event, resource)" 
        "VALUES (%s, %s)")

#acceso al primer documento de excel
wb=openpyxl.load_workbook("C:\Users\Manuel\Desktop\Proyecto_Calendario-master\Modulo BD\PythonCode\HorarioTratado.xlsx")


#acceso a cada una de las páginas que interesan
sheetT2=wb.get_sheet_by_name("Teoria_2")
sheetP2=wb.get_sheet_by_name("PLab_2")

#combinamos los datos semejantes para agilizar el proceso de iteración sobre ellos
cells=sheetT2['A2':'I267']+sheetP2['A2':'I162']


#segundo documento, con los profesores asignados a cada asignatura
wb_profesores=openpyxl.load_workbook('C:\Users\Manuel\Desktop\Proyecto_Calendario-master\profesores_Grado.xlsx')
sheet_mat=wb_profesores.get_sheet_by_name("G-MATEMATICA")
sheet_inf=wb_profesores.get_sheet_by_name("G-INFORMATIC")
sheet_fis=wb_profesores.get_sheet_by_name("G-FISICA")


#combinamos los datos semejantes para agilizar el proceso de iteración sobre ellos
cells_profesores=sheet_mat['A6':'G111']+sheet_inf['A6':'G130']+sheet_fis['A6':'G154']


#una vez tenemos todos los datos podemos introducirlos en la base de datos
for row in cells:
    #flag que marca cuand ohay que anhadir profesores a la asignatura (cuando es nueva)
    tratar=False
    
    #extraemos los datos relevantes
    time = row[0].value
    subject = row[1].value
    id = str(subject)+"_"+str(row[2].value)
    
    #ejecutamos la primera query (registramos cada evento)
    cursor.execute(query1,(id,id,subject,time))
    
    room = row[3].value
    
    #ejecutamos la segunda query si hay que asignar habitacion
    if(room != "" and room !=  None):
        cursor.execute(query2, (id,room))
    
    mat=row[5].value
    doble=row[6].value
    fis=row[7].value
    inf=row[8].value
    
    #se asignan los grupos apropiados a cada evento (clase)
    if(mat==1):
        cursor.execute(query2,(id,'Class1_MATEMATICAS_PLACEHOLDER'))
        tratar=True
    elif (mat==2):
        cursor.execute(query2,(id,'Class2_MATEMATICAS_PLACEHOLDER'))
        tratar=True
    elif (mat==3):
        cursor.execute(query2,(id,'Class3_MATEMATICAS_PLACEHOLDER'))
        tratar=True
    elif (mat==4):
        cursor.execute(query2,(id,'Class4_MATEMATICAS_PLACEHOLDER'))
        tratar=True
    
     
    if(doble==1):
        cursor.execute(query2,(id,'Class1_DOBLE_PLACEHOLDER'))
        tratar=True
    elif (doble==2):
        cursor.execute(query2,(id,'Class2_DOBLE_PLACEHOLDER'))
        tratar=True
    elif (doble==3):
        cursor.execute(query2,(id,'Class3_DOBLE_PLACEHOLDER'))
        tratar=True
    elif (doble==4):
        cursor.execute(query2,(id,'Class4_DOBLE_PLACEHOLDER'))
        tratar=True
    
    if(fis==1):
        cursor.execute(query2,(id,'Class1_FISICA_PLACEHOLDER'))
        tratar=True
    elif (fis==2):
        cursor.execute(query2,(id,'Class2_FISICA_PLACEHOLDER'))
        tratar=True
    elif (fis==3):
        cursor.execute(query2,(id,'Class3_FISICA_PLACEHOLDER'))
        tratar=True
    elif (fis==4):
        cursor.execute(query2,(id,'Class4_FISICA_PLACEHOLDER'))
        tratar=True
        
    if(inf==1):
        cursor.execute(query2,(id,'Class1_INFORMATICA_PLACEHOLDER'))
        tratar=True
    elif (inf==2):
        cursor.execute(query2,(id,'Class2_INFORMATICA_PLACEHOLDER'))
        tratar=True
    elif (inf==3):
        cursor.execute(query2,(id,'Class3_INFORMATICA_PLACEHOLDER'))
        tratar=True
    elif (inf==4):
        cursor.execute(query2,(id,'Class4_INFORMATICA_PLACEHOLDER'))
        tratar=True
   
    #print(profesor+ " "+ prof_corregido)
    
    #comprobar qué profesores hay que asignar a la asignatura
    if (tratar==True):
        for fila in cells_profesores:
            codigo=fila[0].value.replace("(*)","")
            if(codigo==subject):
                profesor= fila[6].value.replace(" ","_").replace(u"\u00F1","n").replace(u"\u00D1","n").lower()
                cursor.execute(query2,(id,profesor))
                
    
    


#ejecutar todo y cerrar cursor y conexion
cnx.commit()
cursor.close()
cnx.close()
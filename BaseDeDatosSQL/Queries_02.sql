#Query que devuelve una SALIDA: todas las clases con id y nombre (events_cuatrimestre2_1718.id y .name) asociadas a una ENTRADA: codigo de asignatura (eventgroups.id)

SELECT events_cuatrimestre2_1718.eventgroup, events_cuatrimestre2_1718.id, events_cuatrimestre2_1718.name FROM  events_cuatrimestre2_1718
	WHERE events_cuatrimestre2_1718.eventgroup='CODIGO_ASIGNATURA_AQUI'; 






#QUERY RESTANTE --> LA INFORMACIÓN ESTÁ EN EL CORREO queries

#1- Mostrar todos los recursos de un tipo concreto -> profesores, clases o aulas

SELECT DISTINCT resources.id, resources.name, resources.resourcetype FROM resources
	WHERE resources.resourcetype='Teacher';

SELECT DISTINCT resources.id, resources.name, resources.resourcetype FROM resources
	WHERE resources.resourcetype='Class';

SELECT DISTINCT resources.id, resources.name, resources.resourcetype FROM resources
	WHERE resources.resourcetype='Room';

#2- Mostrar todas las asignaturas

SELECT eventgroups.id, eventgroups.name, eventgroups.cuatrimestre FROM eventgroups;








#Query que devuelve una SALIDA: todas las clases (events_cuatrimestre2_1718), aulas(resources.id), tiempos (times.id) y profesores (resources.id) asociadas a una ENTRADA: codigo de asignatura (eventgroups.id)

SELECT events_cuatrimestre2_1718.eventgroup, events_cuatrimestre2_1718.id, events_cuatrimestre2_1718.time, res_aulas.name, res_prof.name FROM  events_cuatrimestre2_1718
	INNER JOIN events_need_resources_c2_1718 enr_aulas ON enr_aulas.event= events_cuatrimestre2_1718.id
    INNER JOIN resources res_aulas ON enr_aulas.resource=res_aulas.id
    INNER JOIN events_need_resources_c2_1718 enr_prof ON enr_prof.event= events_cuatrimestre2_1718.id
    INNER JOIN resources res_prof ON enr_prof.resource=res_prof.id
    
    WHERE events_cuatrimestre2_1718.eventgroup='CODIGO_ASIGNATURA_AQUI' 
		AND res_prof.resourcetype='Teacher'
        AND res_aulas.resourcetype='Room';



#Query que con una ENTRADA: tiempo (times.id) devuelve una SALIDA: espacios libres a esa hora 

SELECT DISTINCT res.id, res.name, res.resourcetype, res.color FROM events_cuatrimestre2_1718
	INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event
    INNER JOIN resources res ON enr.resource=res.id
    WHERE res.id NOT IN (
    SELECT DISTINCT res.id #, res.name, res.resourcetype, res.color, events_cuatrimestre2_1718.time 
					FROM events_cuatrimestre2_1718
					INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event
					INNER JOIN resources res ON enr.resource=res.id
					WHERE events_cuatrimestre2_1718.time='TIEMPO_AQUI')
AND res.resourcetype='Room';



#Query que con una ENTRADA: tiempo (times.id), tipo de recurso/espacio (resourcegroups.id)  devuelve una SALIDA: espacios de las caracteristicas deseadas libres a esa hora

SELECT DISTINCT res.id, res.name, res.resourcetype, res.color, rg.id FROM events_cuatrimestre2_1718
	INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event
    INNER JOIN resources res ON enr.resource=res.id
    INNER JOIN resources_to_resourcegroups rtr ON res.id=rtr.resource
    INNER JOIN resourcegroups rg ON rtr.resourcegroup=rg.id 
    WHERE res.id NOT IN (
    SELECT DISTINCT res.id #, res.name, res.resourcetype, res.color, events_cuatrimestre2_1718.time 
					FROM events_cuatrimestre2_1718
					INNER JOIN events_need_resources_c2_1718 enr ON events_cuatrimestre2_1718.id=enr.event
					INNER JOIN resources res ON enr.resource=res.id
					WHERE events_cuatrimestre2_1718.time='TIEMPO_AQUI')
AND res.resourcetype='Room' AND rg.id='RESOURCEGROUP_AQUI';
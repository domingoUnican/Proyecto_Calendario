USE scheduleDB;

-- #mostrar todas las entradas del tipo 'el evento A requiere el recurso B'
-- SELECT * FROM events_need_resources_c2_1718 INNER JOIN events_cuatrimestre2_1718 ON events_need_resources_c2_1718.event=events_cuatrimestre2_1718.id
        -- WHERE events_need_resources_c2_1718.resource="Class1_Matematicas_PLACEHOLDER";        #cambiar por el recurso que se busca
		-- #si se requiere mostrar todos los recursos de determinado tipo, en lugar de esta ultima linea realizar un inner join a 'resources' y filtrar según la columna resourcetype
	
#Mostrar colisiones.	Por defecto se muestran todas, pero se pueden añadir filtros al final para concretar más
SELECT resources.id, resources.name, ecA.name as 'Evento A', ecB.name as 'Evento B', ecA.time FROM resources
	INNER JOIN events_need_resources_c2_1718 enrA ON resources.id=enrA.resource
    INNER JOIN events_cuatrimestre2_1718 ecA ON ecA.id=enrA.event
    INNER JOIN events_need_resources_c2_1718 enrB ON resources.id=enrB.resource
    INNER JOIN events_cuatrimestre2_1718 ecB ON enrB.event=ecB.id
    WHERE ecA.id!=ecB.id AND ecA.time=ecB.time -- AND resources.resourcetype='' #aquí se puede restringir el tipo de recurso o, con pocos cambios, el recurso concreto
    ;
    
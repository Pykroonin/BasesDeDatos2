/*
* Created by roberto on 3/5/21.
*/
#include <stdio.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "search.h"

void    results_search(char * from, char *to,char * date,
                       int * n_choices, char *** choices,
                       int max_length,
                       int max_rows)
   /**here you need to do your query and fill the choices array of strings
 *
 * @param from form field from
 * @param to form field to
 * @param n_choices fill this with the number of results
 * @param choices fill this with the actual results
 * @param max_length output win maximum width
 * @param max_rows output win maximum number of rows
  */
{
    int i=0;
    int t=0;
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret; /* ODBC API return status */

    SQL_TIMESTAMP_STRUCT fechita;
    SQL_TIMESTAMP_STRUCT fechita2;
    unsigned short int year;
    unsigned short int month;
    unsigned short int day;
    SQLCHAR fecha[11];

    SQLCHAR flight_id[512];
    SQLCHAR scheduled_departure[64]; 
    SQLCHAR scheduled_arrival[64]; 
    SQLCHAR transbordos[64];
    SQLCHAR asientos_vacios[64];
    SQLCHAR tiempo[64];
    SQLCHAR final_array[512];
    SQLCHAR space[2] = " ";

    SQLCHAR parte1[] = "WITH asientos_totales AS (SELECT f.flight_id, Count (s.seat_no) AS totales FROM flights f, aircrafts_data ad, seats s WHERE f.aircraft_code = ad.aircraft_code AND ad.aircraft_code = s.aircraft_code and f.scheduled_departure > ? and f.scheduled_departure < ? GROUP BY f.flight_id),";
    SQLCHAR parte2[] = "asientos_ocupados AS (SELECT f.flight_id, Count (tf.ticket_no) AS ocupados FROM flights f, ticket_flights tf WHERE tf.flight_id = f.flight_id GROUP BY f.flight_id),";
    SQLCHAR parte3[] = "asientos_vacios AS (SELECT t.flight_id, ( t.totales - o.ocupados ) AS vacios FROM asientos_totales t, asientos_ocupados o WHERE t.flight_id = o.flight_id),";
    SQLCHAR parte4[] = "vuelos_salida as ( select f.flight_id, f.scheduled_departure, f.scheduled_arrival, f.arrival_airport from flights f where f.departure_airport=?),";
    SQLCHAR parte5[] = "vuelos_llegada as ( select f.flight_id, f.scheduled_departure, f.departure_airport, f.scheduled_arrival from flights f where f.arrival_airport=?)";
    SQLCHAR parte6[] = "select flight_id, scheduled_departure, scheduled_arrival , transbordos, asientos_vacios, tiempo_de_viaje from (select vs.flight_id, vs.scheduled_departure, vs.scheduled_arrival, 0 as transbordos, av.vacios as asientos_vacios, (vs.scheduled_arrival - vs.scheduled_departure) as tiempo_de_viaje from vuelos_salida vs, asientos_vacios av where vs.arrival_airport = ? ";
    SQLCHAR parte7[] = "and av.flight_id = vs.flight_id and av.vacios <> 0 union select vs.flight_id, vs.scheduled_departure, vl.scheduled_arrival, 1 as transbordos, min(av.vacios) as asientos_vacios, (vl.scheduled_arrival - vs.scheduled_departure) as tiempo_de_viaje from vuelos_salida vs, vuelos_llegada vl, asientos_vacios av where vl.departure_airport = vs.arrival_airport and vs.scheduled_arrival < vl.scheduled_departure and vl.scheduled_arrival - vs.scheduled_departure < '24:00:00.000' and ";
    SQLCHAR parte8[] = "(av.flight_id = vs.flight_id or av.flight_id = vl.flight_id) and 0 <> (select min(av.vacios) from asientos_vacios av where (av.flight_id = vs.flight_id or av.flight_id = vl.flight_id)) group by vs.flight_id, vs.scheduled_departure, vl.scheduled_arrival) as t order by tiempo_de_viaje";
    SQLCHAR consulta_completa[8192]; 

    strcpy((char*)consulta_completa, (char*)parte1);
    strcat((char*)consulta_completa, (char*)parte2);
    strcat((char*)consulta_completa, (char*)parte3);
    strcat((char*)consulta_completa, (char*)parte4);
    strcat((char*)consulta_completa, (char*)parte5);
    strcat((char*)consulta_completa, (char*)parte6);
    strcat((char*)consulta_completa, (char*)parte7);
    strcat((char*)consulta_completa, (char*)parte8);


    ret = odbc_connect(&env, &dbc);

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    
    SQLPrepare(stmt, consulta_completa, SQL_NTS);
     /*SQLPrepare(stmt, (SQLCHAR*)"select * from flights where scheduled_departure > ? and scheduled_arrival < ? and departure_airport = ?  and arrival_airport=? limit 10", SQL_NTS);*/
    sscanf(date, "%4hu_%2hu_%2hu", &year, &month, &day);
        fechita.year = year;
        fechita.month = month;
        fechita.day = day;
        fechita.hour = 0;
        fechita.minute = 0;
        fechita.second = 0;
        fechita.fraction = 0;

        fechita2.year = fechita.year;
        fechita2.month = fechita.month;
        fechita2.day = fechita.day+1;
        fechita2.hour = 0;
        fechita2.minute = 0;
        fechita2.second = 0;
        fechita2.fraction = 0;
    /*strncpy((char*)departure_airport, "SVO", 4);*/
    /*strncpy((char*)arrival_airport, "LED", 4);*/

    /*printf("%s / %s / %hu-%hu-%hu /",departure_airport, arrival_airport, year, month, day );*/

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &fechita, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &fechita2, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)from), 0, (SQLCHAR*)from, sizeof((SQLCHAR*)from), NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)to), 0, (SQLCHAR*)to, sizeof((SQLCHAR*)to), NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)to), 0, (SQLCHAR*)to, sizeof((SQLCHAR*)to), NULL);
        
    SQLExecute(stmt);
        
    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_id, sizeof(flight_id), NULL);
    SQLBindCol(stmt, 2, SQL_C_CHAR, scheduled_departure, sizeof(scheduled_departure), NULL);
    SQLBindCol(stmt, 3, SQL_C_CHAR, scheduled_arrival, sizeof(scheduled_arrival), NULL);
    SQLBindCol(stmt, 4, SQL_C_CHAR, transbordos, sizeof(transbordos), NULL);
    SQLBindCol(stmt, 5, SQL_C_CHAR, asientos_vacios, sizeof(asientos_vacios), NULL);
    SQLBindCol(stmt, 6, SQL_C_CHAR, tiempo, sizeof(tiempo), NULL);

    *n_choices = 9 ;/*Hay que cambiar esto*/
    max_rows = MIN(*n_choices, max_rows);

    while (SQL_SUCCEEDED(ret = SQLFetch(stmt)) && i < max_rows) {
    
        strcpy((char*)final_array, (char*)flight_id); 
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)scheduled_departure);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)scheduled_arrival);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)transbordos);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)asientos_vacios);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)tiempo);
        
        t = strlen((char*)final_array)+1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], (char*)final_array, t);
        i++;
    }

    SQLCloseCursor(stmt);

       /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);



}


## Práctica 3 - Sistema de venta online en cines
### Objetivo
Se solicita el desarrollo de un sistema online de venta para cines, tanto para la adquisición de tickets como para la compra de bebida y palomitas. Supongamos que el cine está construido por una única sala de 72 asientos. 
Por cada cliente se debe generar un hilo que represente su solicitud (el control de turnos de los clientes se debe modelar mediante una cola FIFO). 
La generación de cada petición de cliente se realizará de forma aleatoria y constará de los siguientes elementos: 

**Solicitud entradas: <identificador_cliente> <numero_asientos>**

Si no quedan entradas suficientes el cliente se irá del cine. 

Una vez seleccionados los asientos se debe proceder al pago. El **sistema de pago** por tarjeta es un recurso compartido por los clientes en el que se debe garantizar **exclusión mutua**. Para evitar inconsistencias en la asignación de asientos y pago de entradas, ambos recursos deberán ser adquiridos de forma simultánea. Finalmente, cuando un cliente cuente con sus asientos y haya realizado el pago, debe liberar los recursos para que otro cliente pueda elegir asientos o bien otro cliente pueda efectuar el pago de la bebida y palomitas. 

Una vez que el cliente dispone de sus entradas, la siguiente fase consiste en la **compra de bebida y palomitas**. Para la compra de alimentos existen **tres puntos de venta y una sola cola de espera**. Un cliente hará uso de un punto de venta cuando esté libre. Cada punto de venta tiene un número limitado de bebidas y palomitas; cuando éstas se acaban será necesario despertar a un hilo **reponedor** cuyo acceso debe ser exclusivo al solo existir uno, encargado de restaurar el número de bebidas y palomitas en el punto de venta. Mientras el reponedor hace su trabajo, el cliente deberá esperar en el punto de venta. Cuando el reponedor finalice debe notificar al cliente para continuar su compra. La petición de un cliente en esta segunda fase consta de los siguientes campos: 

**Solicitud bebidas/palomitas: <identificador_cliente> <numero_bebidas> <numero_palomitas>**

Por último, el cliente deberá esperar a que el sistema de pag esté libre para efectuar el pago. Mientras un cliente efectúa el pago de sus bebidas y palomitas ningún otro cliente podrá hacer ningún pago. Debido a esto el sistema debe gestionar un **mecanismo de prioridades** que otorgue con mayor frecuencia el sistema de pago a los cientes que quieren adquirir asientos, es decir, se da mayor prioridad al acceso al cine que a la compra de bebidas y palomitas.
Tras el pago, el cliente entra a la sala y el hilo que representa su solicitud debe finalizar. 

Tanto en la fase I (compra de tickets) como en la fase II (compra de bebidas y palomitas) se deberá mostrar la información relativa al cliente y sus peticiones. 


### Compilación del código
Para compilar se escribe en el terminal 
```shell
make all
```

### Ejecución del código 
Para ejecutar se escribe en el terminal 
```shell
make run
``` 
El comienzo del programa sería el siguiente: 
![Texto alternativo](/img/run.png)

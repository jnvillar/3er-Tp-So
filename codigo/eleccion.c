#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"

static t_pid siguiente_pid(t_pid pid, int es_ultimo)
{
 t_pid res= 0; /* Para silenciar el warning del compilador. */

 if (es_ultimo)
	res= 1;
 else
	res= pid+1;

 return res;
}


void iniciar_eleccion(t_pid pid, int es_ultimo){
 /* Completar acá el algoritmo de inicio de la elección.
  * Si no está bien documentado, no aprueba.
  */

	int par[2];
	par[0] = pid;
	par[1] = pid;
	MPI_Request a;
	MPI_Status b;
	double tiempoEsperaMaximo = 0.5;

  	int i = 0;
	int flag = 0;
	while(flag != 1){		// mando mensaje a mi siguiente y espero su confirmacion. Mientras no reciba confirmacion:
		MPI_Isend(&par,2,MPI_INT,siguiente_pid(pid,es_ultimo)+i,0,MPI_COMM_WORLD,&a); 		// mando al siguiente proceso vivo el mensaje
		double ahoraMsg = 0;
		while(ahoraMsg < tiempoEsperaMaximo){ 		// espero confirmacion (durante un tiempo)
			MPI_Iprobe(siguiente_pid(pid,es_ultimo)+i,1,MPI_COMM_WORLD,&flag,&b);		// veo si el destinatario me confirma la recepcion
			if (flag == 1){
				MPI_Irecv(&par,2,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&a);	// desencola el mensaje
				break;															// si me confirman salgo del ciclo
			}
			ahoraMsg += MPI_Wtime();
		}
		i++;										// si no me confirmo (flag != 1) entonces asumo que esta muerto y busco confirmacion del nuevo siguiente
	}

}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout)
{
 static t_status status= NO_LIDER;
 double ahora= MPI_Wtime();
 double tiempo_maximo= ahora+timeout;
 t_pid proximo= siguiente_pid(pid, es_ultimo);
 double tiempoEsperaMaximo = 0.5;
 while (ahora<tiempo_maximo){
	 /* Completar acá el algoritmo de elección de líder.
	  * Si no está bien documentado, no aprueba.
          */
	int par[2];
	par[0] = -1;

  	MPI_Request a;
  	MPI_Status b;
  	int flag;
  	MPI_Iprobe(MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&flag,&b);		// se fija si le llego algun mensaje de cualquier proceso
  		
	if (flag == 1){												// si le llego, entonces
		MPI_Isend(&par,2,MPI_INT,b.MPI_SOURCE,1,MPI_COMM_WORLD,&a);		// envia su confirmacion
		MPI_Irecv(&par,2,MPI_INT,MPI_ANY_SOURCE,0 ,MPI_COMM_WORLD,&a);	// recibe el mensaje
		if (pid == par[0]){ 		// si dio una vuelta entera (o sea, pid comenzo la eleccion)
			if (pid == par[1]){		// si el id maximo es el suyo
				status = LIDER;			// es lider
			} else {
				par[0] = par[1];	// para que el lider sepa que es el
			}
		} else{
			if (par[1]<pid){	
				par[1] = pid;		// actualizo el id maximo
			}
		}

		int i = 0;
		flag = 0;
		while(flag != 1){		// mando mensaje a mi siguiente y espero su confirmacion. Mientras no reciba confirmacion:
			printf("%d\n", siguiente_pid(pid,es_ultimo)+i);
			MPI_Isend(&par,2,MPI_INT,siguiente_pid(pid,es_ultimo)+i,0,MPI_COMM_WORLD,&a); 		// mando al siguiente proceso vivo el mensaje
			double ahoraMsg = 0;
			while(ahoraMsg < tiempoEsperaMaximo){ 		// espero confirmacion (durante un tiempo)
				MPI_Iprobe(siguiente_pid(pid,es_ultimo)+i,1,MPI_COMM_WORLD,&flag,&b);		// veo si el destinatario me confirma la recepcion

				if (flag == 1){
					MPI_Irecv(&par,2,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&a);	// desencola el mensaje
					break;															// si me confirman salgo del ciclo
				}
				ahoraMsg += MPI_Wtime();
			}
			i++;										// si no me confirmo (flag != 1) entonces asumo que esta muerto y busco confirmacion del nuevo siguiente
		}
			
	

	}

	


	 /* Actualizo valor de la hora. */
	 ahora= MPI_Wtime();
}

 /* Reporto mi status al final de la ronda. */
 printf("Proceso %u %s líder.\n", pid, (status==LIDER ? "es" : "no es"));
}

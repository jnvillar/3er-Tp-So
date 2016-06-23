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
  	//printf("Inicia elección: %d\n", pid);
  	t_pid proximo= siguiente_pid(pid, es_ultimo);
	int par[2];
	par[0] = pid;
	par[1] = pid;
	MPI_Request req;
	MPI_Request req2;

	MPI_Status b;
	double timeoutMsg = 0.5;

	int mandarAck = TRUE;
  	int i = 0;
	int flag = 0;
	int flag2 = 0;
	while(flag != 1){																				// Mientras no reciba confirmacion:
		//printf("Envio mensaje: %d->%d\n", pid,siguiente_pid(pid,es_ultimo)+i);
		MPI_Isend(&par,2,MPI_INT,proximo+i,TAG_MSG,MPI_COMM_WORLD,&req); 			// Mando al siguiente proceso vivo el mensaje
		double ahoraMsg = MPI_Wtime();
		double tiempoEsperaMaximo = ahoraMsg + timeoutMsg;
		while(ahoraMsg < tiempoEsperaMaximo){ 															// Espero confirmacion (durante un tiempo):
			MPI_Iprobe(proximo+i,TAG_ACK,MPI_COMM_WORLD,&flag,&b);						// Veo si el destinatario me confirma la recepcion
			if (flag == 1){																					// Si me llego confirmacion:
				//printf("Recibi confirmacion: %d->%d\n",siguiente_pid(pid,es_ultimo)+i,pid);
				MPI_Irecv(&par,2,MPI_INT,proximo+i,TAG_ACK,MPI_COMM_WORLD,&req2);			// Desencola el mensaje
				break;																							// Salgo del ciclo (y de la funcion)
			} else {
				//printf("No recibi confirmacion: %d->%d\n",siguiente_pid(pid,es_ultimo)+i,pid);
			}
			
			
			if (mandarAck){																				// Para mandar la confirmacion una sola vez
				MPI_Iprobe(MPI_ANY_SOURCE,TAG_MSG,MPI_COMM_WORLD,&flag2,&b);		
				if (flag2 == 1){																			// Si alguien esta esperando que le confirme la recepion:
					MPI_Isend(&par,2,MPI_INT,b.MPI_SOURCE,TAG_ACK,MPI_COMM_WORLD,&req);							// Mando confirmacion
					mandarAck = FALSE;																			// Como no desencolo el mensaje (lo hare en eleccion_lider), flag2 sera siempre 1. Para no mandar confirmaciones de más, seteo mandarAck como FALSE 
				}
			}

			
			ahoraMsg = MPI_Wtime();																		// Actualizo tiempo.
		}
		i++;
		//if(flag!=1)printf("No recibi confirmacion: %d->%d\n",siguiente_pid(pid,es_ultimo)+i,pid); 										
	}

}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout){

	static t_status status= NO_LIDER;	
	double ahora= MPI_Wtime();
	double tiempo_maximo= ahora+timeout;
	t_pid proximo= siguiente_pid(pid, es_ultimo);
	double timeoutMsg = 0.5;
	int max = -1;

	while (ahora<tiempo_maximo){
		 /* Completar acá el algoritmo de elección de líder.
		  * Si no está bien documentado, no aprueba.
		      */
		int par[2];																			// par[0] es el id del iniciador. par[1] es el candidato a lider hasta el momento
		par[0] = -1;

	  	MPI_Request req;
	  	MPI_Request req2;
	  	MPI_Status b;
	  	int flag = 0;
	  	int flag2 = 0;
	  	int j = 0;
	  	int mandarAck = TRUE;
	  	//printf("Espero mensaje: %d\n",pid);
		MPI_Iprobe(MPI_ANY_SOURCE,TAG_MSG,MPI_COMM_WORLD,&flag,&b);							// Se fija si le llego algun mensaje de cualquier proceso
	  	
		if (flag == 1){																		// Si le llego, entonces:
			//printf("Recibi mensaje. Envio confirmacion: %d->%d\n", pid,b.MPI_SOURCE);
			MPI_Isend(&par,2,MPI_INT,b.MPI_SOURCE,TAG_ACK,MPI_COMM_WORLD,&req);					// Envia su confirmacion
			MPI_Irecv(&par,2,MPI_INT,MPI_ANY_SOURCE,TAG_MSG ,MPI_COMM_WORLD,&req2);				// Recibe el mensaje
			if (par[1] < max){																	// Si ya habia recibido un mensaje con mayor candidato a lider:
				j = 1;																				// Vuelve a esperar por un mensaje
			} else {
				max = par[1];
				if (pid == par[0]){ 															// Si dio una vuelta entera (o sea, pid comenzo la eleccion):
					if (pid == par[1]){																// Si el id maximo es el suyo:
						status = LIDER;																	// Es lider
						j = 1;																			// Vuelve a esperar por un mensaje
					} else {
						par[0] = par[1];															// Para que el lider sepa que es él
					}
				} 
				else{
					if (par[1]<pid){	
						par[1] = pid;															// Actualizo el id maximo
					}
				}
			}
			int i = 0;
			flag = j;
			while(flag != 1){																	// Mientras no reciba confirmacion:
				//printf("Envio mensaje: %d->%d\n", pid,siguiente_pid(pid,es_ultimo)+i);
				MPI_Isend(&par,2,MPI_INT,proximo+i,TAG_MSG,MPI_COMM_WORLD,&req); 		// Mando al siguiente proceso vivo el mensaje
				double ahoraMsg = MPI_Wtime();
				double tiempoEsperaMaximo = ahoraMsg + timeoutMsg;
				while(ahoraMsg < tiempoEsperaMaximo){ 														// Espero confirmacion (durante un tiempo):
					MPI_Iprobe(proximo+i,TAG_ACK,MPI_COMM_WORLD,&flag,&b);					// Veo si el destinatario me confirma la recepcion
					if (flag == 1){																				// Si me llego confirmacion:
						//printf("Recibi confirmacion: %d->%d\n",siguiente_pid(pid,es_ultimo)+i,pid);
						MPI_Irecv(&par,2,MPI_INT,proximo+i,TAG_ACK,MPI_COMM_WORLD,&req2);			// Desencola el mensaje
						break;																							// Salgo del ciclo
					} else {
						//printf("No recibi confirmacion: %d->%d\n",siguiente_pid(pid,es_ultimo)+i,pid);
					}
					
					if (mandarAck){																			// Para mandar la confirmacion una sola vez
						MPI_Iprobe(MPI_ANY_SOURCE,TAG_MSG,MPI_COMM_WORLD,&flag2,&b);
						if (flag2 == 1){																	// Si alguien esta esperando que le confirme la recepion:
							MPI_Isend(&par,2,MPI_INT,b.MPI_SOURCE,TAG_ACK,MPI_COMM_WORLD,&req);					// Mando confirmacion
							mandarAck = FALSE;																	// Como no desencolo el mensaje (lo hare en la proxima iteracion), flag2 sera siempre 1. Para no mandar confirmaciones de más, seteo mandarAck como FALSE 
						}
					}

					ahoraMsg = MPI_Wtime();																	// Actualizo tiempo.
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

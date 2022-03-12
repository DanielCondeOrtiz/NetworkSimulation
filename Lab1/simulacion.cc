/*************************************************************************/
/**		PRÁCTICA 1 DE PLANIFICACIÓN Y SIMULACIÓN DE REDES	**/
/**								      	**/
/**			SIMULACION.CC					**/
/**								     	**/
/*************************************************************************/

#include <ns3/core-module.h>
#include <ns3/gnuplot.h>

#include "llegadas.h"

#define MEDIA    10
#define MUESTRAS 20
#define FIN      1000
// Valor de la t-Student para el 90% y 20 muestras
#define TSTUDENT 1.729 

using namespace ns3;

// Se declara el nombre del registro (componente de trazado) para este módulo de código.
// En este caso el identificador será "Simulacion"
NS_LOG_COMPONENT_DEFINE ("Simulacion");

int main (int argc, char *argv[])
{ 
  // Se establece resolución de ms
  Time::SetResolution (Time::MS);
  // Se declara un objeto Time que guardará el tiempo de duración de la simulación
  Time stop = Seconds (FIN); 
  // Se establecen los valores por defecto de las variables
  double tasainicial = MEDIA;
  
  CommandLine cmd;
  cmd.AddValue ("duracion", "Duración de la simulación", stop);
  cmd.AddValue ("tasaMedia", "Tasa media del proceso de poisson", tasainicial);
  
  cmd.Parse (argc, argv);

  // Definimos el modelo a simular
  Llegadas modelo (tasainicial);
  // Programo la finalización de la simulación.
  Simulator::Stop (stop);

  // Lanzamos la simulación.
  NS_LOG_INFO ("GESTION DE LA SIMULACION: se inicia la simulación en el instante virtual " << Simulator:: Now());
  Simulator::Run ();
  NS_LOG_INFO ("GESTION DE LA SIMULACION: termina la simulación en el instante virtual " << Simulator:: Now());
  Simulator::Destroy ();
  NS_LOG_INFO ("GESTIÓN DE LA SIMULACIÓN: tras Destroy el instante virtual es " << Simulator::Now ());
}

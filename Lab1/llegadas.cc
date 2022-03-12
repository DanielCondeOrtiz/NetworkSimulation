/*************************************************************************/
/**  PRÁCTICA 1 DE PLANIFICACIÓN Y SIMULACIÓN DE REDES			**/
/**								      	**/
/**				LLEGADAS.CC				**/
/**								     	**/
/*************************************************************************/


#include <ns3/core-module.h>

#include "llegadas.h"

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Llegadas");

////////////////////////////////////////////////////////////////////////////////
//
// Modelo a simular: patrón de llegadas según Poisson de tasa determinada
//


Llegadas::Llegadas (double tasa)
{
  NS_LOG_FUNCTION (tasa);

  // Creamos la variable aleatoria ...
  tEntreLlegadas = CreateObject<ExponentialRandomVariable> ();

  DoubleValue media;
  tEntreLlegadas->GetAttribute ("Mean", media);
  NS_LOG_INFO ("GESTIÓN DE ATRIBUTOS: por defecto el atributo del tipo ExponentialRandomVariable es :"
               << media.Get () << ". La cambiamos a " << 1/tasa);
  // ... y ajustamos su valor medio
  tEntreLlegadas->SetAttribute ("Mean", DoubleValue (1/tasa));
  tEntreLlegadas->GetAttribute ("Mean", media);
  NS_LOG_INFO ("GESTIÓN DE ATRIBUTOS: Tras usar SetAttribute, la media del objeto es :" << media.Get ());
  /* Alternativa
     // Especifico el valor de la media directamente al crear el objeto
     tEntreLlegadas = CreateObject<ExponentialRandomVariable> (1/tasa);
  */

  // Programamos la primera llegada.
  Time dentrode = Seconds(tEntreLlegadas->GetValue ());
  
  Simulator::Schedule (dentrode,
                       &Llegadas::NuevaLlegada, this,
                       Simulator::Now ());
  
  // Indico que se pase como parámetro el momento en el que se está programando el evento, es decir ahora
  NS_LOG_INFO ("GESTIÓN DE EVENTOS: se programa el primer evento en el constructor de la clase Llegadas. En el instante simulado "
               << Simulator::Now () << " para dentro de " << dentrode);
					  
}

void
Llegadas::NuevaLlegada (Time tiempo)
// El parámetro recibido es el instante de la llegada anterior, cuando se programó la que se está gestionando ahora
{
  NS_LOG_FUNCTION (tiempo);
  // Calculamos el tiempo transcurrido desde la última llegada
  Time intervalo = Simulator::Now () - tiempo;

  // Presentamos el tiempo transcurrido entre llegadas.
  // Imprimimos el tiempo transcurrido desde el último evento.
  NS_LOG_DEBUG ("Nuevo evento en " << Simulator::Now() << " tras " << intervalo);

  // Programamos la siguiente llegada
  Time dentrode = Seconds(tEntreLlegadas->GetValue());
  Simulator::Schedule(dentrode,
                      &Llegadas::NuevaLlegada, this,
                      Simulator::Now ());
  NS_LOG_INFO("GESTIÓN DE EVENTOS: se programa el siguiente evento en el método NuevaLlegada. En el instante simulado "
              << Simulator::Now () << " para dentro de " << dentrode);
  // Acumulamos el valor del intervalo
  this->acumulador.Update (intervalo.GetSeconds ());

}

double Llegadas::Media ()
{
  NS_LOG_FUNCTION_NOARGS();
  return acumulador.Mean ();
}

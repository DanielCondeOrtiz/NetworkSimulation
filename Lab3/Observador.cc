#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

//Clase Observador para obtener los valores necesitados de cada uno de los nodos
Observador::Observador (Ptr<CsmaNetDevice> device_ptr)
{
  m_numIntentosTx=0;				//Variable para acumular el numero medio de intentos de transmision de un paquete	
  m_numPaqPerdidos=0;				//Variable para contar el numero de paquetes perdidos
  m_miDevice_ptr=device_ptr;			
  ProgramaTrazas ();				
}

/*
Metodo: ProgramaTrazas
Descripcion: Respecto a las trazas que se quieren observar, se asigna a cada una de ellas la función de callback a la que deberán llamar cuando ocurran.
*/
void
Observador::ProgramaTrazas ()
{
 m_miDevice_ptr->TraceConnectWithoutContext ("MacTxBackoff", MakeCallback (&Observador::IntentoEnvio, this));		//Cuando se intenta enviar un paquete se ejecuta el metodo IntentoEnvio 
 m_miDevice_ptr->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&Observador::PaqueteEntregado, this));		//Cuando termina de transmitirse un paquete se invoca el metodo PaqueteEntregado
 m_miDevice_ptr->TraceConnectWithoutContext ("PhyTxDrop", MakeCallback (&Observador::PaqueteDescartado, this));		/*Cuando se descarta un paquete en la capa fisica porque el medio esta ocupado
 																												      se ejecuta el método PaqueteDescartado */
 m_miDevice_ptr->TraceConnectWithoutContext ("MacTx", MakeCallback (&Observador::PaqueteBaja, this));				//Cuando la capa MAC transmite un paquete de la capa superior, se invoca el método PaqueteBaja
 m_miDevice_ptr->TraceConnectWithoutContext ("MacRx", MakeCallback (&Observador::PaqueteSube, this));				//Cuando la capa MAC recibe un paquete de la capa inferior, se ejecuta el metodo PaqueteSube

}

/*
Metodo: IntentoEnvio
Descripcion: Aumenta la variable que cuenta el numero de intentos de un paquete
*/
void
Observador::IntentoEnvio (Ptr<const Packet> pqt)
{
  m_numIntentosTx++;						
}

/*
Metodo: PaqueteEntregado
Descripcion: Cuando el paquete se envia, aumentamos el numero de intentos, pues el intento correcto tambien cuenta, actualizamos la variable Average y reseteamos
			 el valor de numIntentosTx para el siguiente paquete
*/
void
Observador::PaqueteEntregado (Ptr<const Packet> pqt)
{
  m_numIntentosTx++;
  m_numIntentos.Update (m_numIntentosTx);
  m_numIntentosTx=0;
}

/*
Metodo:PaqueteDescartado
Descripcion: Cuando descartamos un paquete, ponemos la variable numIntentosTx a 0 para el siguiente paquete y aumentamos el numero de paquetes perdidos por el nodo
*/
void
Observador::PaqueteDescartado (Ptr<const Packet> pqt)
{
  m_numIntentosTx=0;
  m_numPaqPerdidos++;  
}

/*
Metodo: GetIntentosMedios
Descripcion: devuelve la media de la variable Average que acumula el numero de intentos de cada paquete
*/
double Observador::GetIntentosMedio ()
{
  return m_numIntentos.Mean();
}

/*
Metodo: GetIntentosCorrectos
Descripcion: Devuelve el numero de elementos de la variable Average, la cual es como un vector en la que cada valor es el numero de intentos
			 de un paquete enviado correctamente
*/
double Observador::GetIntentosCorrectos()
{
  return m_numIntentos.Count();
}		

/*
Metodo: GetPaqPerdidos
Descripcion: Devuelve el porcentaje (%) de paquetes perdidos
*/
double Observador::GetPaqPerdidos ()
{
  return 100*m_numPaqPerdidos/(m_numPaqPerdidos + m_numIntentos.Count());
}		

/*
Metodo:PaqueteBaja
Descripcion:Obtiene el instante en el que un paquete es recibido del protocolo superior y está listo para enviar
*/
void
Observador::PaqueteBaja (Ptr<const Packet> pqt)
{
  m_instanteRxAp=Simulator::Now ();
}

/*
Metodo: PaqueteSube
Descripcion: Calcula el retardo a partir del instante en el que un paquete es recibido y listo para transmision a protocolo superior,
			 tambien actualiza el valor de la variable Average que reune el valor de retardo de todos los ecos enviados
*/
void
Observador::PaqueteSube (Ptr<const Packet> pqt)
{
  Time retardo=Simulator::Now ()-m_instanteRxAp;
  if (retardo > 0)
    m_retardos.Update (retardo.GetDouble ());
}

/*
Metodo: GetRetardoMedio
Descripcion: Devuelve la media del retardo de todos los ecos enviados
*/
double Observador::GetRetardoMedio ()
{
  return m_retardos.Mean ();
}
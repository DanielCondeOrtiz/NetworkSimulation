/*
Practica 04 - Simulación de Redes
Fecha: 19/11/17
Alumnos: Daniel Conde Ortiz
		 Miguel Ruiz Ramos

Fichero: Observador.cc
Descripcion: Este fichero contiene el código de las clases Observador y MiTag, necesarias para observar el escenario de la práctica 4 y obtener los
			 resultados estadisticos.
*/

#include "Observador.h"
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/tag-buffer.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

//Clase Observador para obtener los valores necesitados de cada uno de los nodos
Observador::Observador (NetDeviceContainer * csmaDevices, Ptr<PointToPointNetDevice> device_sumidero, ApplicationContainer * sumideroapp)
{	
	NS_LOG_INFO("Observador creado");
	enviados=0;
	recibidos=0;
	m_csmaDevices = csmaDevices;
	m_sumidero_ptr=device_sumidero;	
	m_sumideroapp = sumideroapp;
	ProgramaTrazas ();
	ultimo = Simulator::Now();	
	Simulator::Schedule(Seconds(3), &Observador::Comprueba, this);

}

/*
Metodo: ProgramaTrazas
Descripcion: Respecto a las trazas que se quieren observar, se asigna a cada una de ellas la función de callback a la que deberán llamar cuando ocurran.
*/
void
Observador::ProgramaTrazas ()
{
	// Dispositivo al que se están añadiento los callback para las trazas
	Ptr<CsmaNetDevice> m_fuente;
	// Numero de nodos en el bus Csma
	uint32_t numNodos = m_csmaDevices->GetN();
	
	NS_LOG_DEBUG("Numero de nodos(nCsma): " << numNodos);
	
	for(uint8_t j = 1; j < numNodos; j++) 	//En el nodo 0 (csma) está el puente, por eso j=1
	{
		m_fuente=m_csmaDevices->Get (j)->GetObject<CsmaNetDevice> ();
 		m_fuente->TraceConnectWithoutContext ("MacTx", MakeCallback (&Observador::PaqueteBaja, this));
 		NS_LOG_DEBUG("Traza situada en el nodo " << j << " del bus csma");
 	}

	m_sumidero_ptr->TraceConnectWithoutContext ("MacRx", MakeCallback (&Observador::PaqueteSube, this));
	NS_LOG_DEBUG("Traza en el sumidero");
	NS_LOG_INFO("Trazas programadas");
}	

/*
Metodo:PaqueteBaja
Descripcion:Funcion callback para cuando se envia un paquete.
			Añade una etiqueta al paquete con el instante en el que fue enviado.
*/
void Observador::PaqueteBaja (Ptr<const Packet> pqt)
{
	MiTag etiqueta;
	etiqueta.SetTimestamp(Simulator::Now ());
	NS_LOG_DEBUG("Paquete enviado en " << Simulator::Now());
	pqt->AddPacketTag(etiqueta);
	enviados++;
}

/*
Metodo: PaqueteSube
Descripcion:Funcion callback para cuando se recibe un paquete.
			Lee la etiqueta de tiempo y actualiza el valor
			de la variable Average que reune el valor de retardo de todos los paquetes.
*/
void Observador::PaqueteSube (Ptr<const Packet> pqt)
{
	MiTag tagCopy;
	pqt->PeekPacketTag (tagCopy);
	
	recibidos++;
	Time retardo= Simulator::Now () - tagCopy.GetTimestamp();
	NS_LOG_DEBUG("Retardo del paquete: " << retardo);
	m_retardos.Update (retardo.GetMilliSeconds());
	
	ultimo = Simulator::Now();	


}

/*
Metodo: Comprueba
Descripcion: Esta funcion nos sirve para comprobar si han pasado X (por defecto 5) segundos
			 desde la última vez que se recibió un paquete. Si es así, se para el sumidero; si no,
			 se programa la función para dentro de 5 segundos.
*/
void Observador::Comprueba()
{

	if((Simulator::Now()).GetSeconds() - ultimo.GetSeconds() > 5)
	{
		m_sumideroapp->Stop(Simulator::Now() + MilliSeconds(10));
		NS_LOG_DEBUG("Programado el apagado del sumidero");
	}
	else
	{
		Simulator::Schedule(Seconds(5), &Observador::Comprueba, this);
	}
}

/*
Metodo: GetRetardoMedio
Descripcion: Devuelve la media del retardo de todos los paquetes enviados
*/
double Observador::GetRetardoMedio ()
{
	NS_LOG_INFO("La media obtenida es:" << m_retardos.Mean());
	return m_retardos.Mean ();
}

/*
Metodo: GetPaqCorrectos
Descripcion: Devuelve el porcentaje de paquetes enviados correctamente por el nodo
*/
double Observador::GetPaqCorrectos ()
{
	NS_LOG_INFO("El porcentaje de paquetes es: "<< recibidos/enviados * 100);
	return recibidos/enviados * 100;
}



//----------------------------------------------------------------------
//-- MiTag
//-- Métodos que implementa la clase MiTag
//-- Nosotros usaremos GetTimestamp() y SetTimestam() para
//-- leer y escribir en el paquete el instante de envio.
//-- Esta clase ha sido copiada del fichero /src/internet/test/tcp-timestamp-test.cc
//-- y renombrada con el nombre MiTag
//------------------------------------------------------
TypeId MiTag::GetTypeId (void)
{
	static TypeId tid = TypeId ("MiTag")
	.SetParent<Tag> ()
	.AddConstructor<MiTag> ()
	.AddAttribute ("Timestamp",
		"Some momentous point in time!",
		EmptyAttributeValue (),
		MakeTimeAccessor (&MiTag::GetTimestamp),
		MakeTimeChecker ())
	;
	return tid;
}

TypeId MiTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}

uint32_t MiTag::GetSerializedSize (void) const
{
	return 8;
}

void MiTag::Serialize (TagBuffer i) const
{
	int64_t t = m_timestamp.GetNanoSeconds ();
	i.Write ((const uint8_t *)&t, 8);
}

void MiTag::Deserialize (TagBuffer i)
{
	int64_t t;
	i.Read ((uint8_t *)&t, 8);
	m_timestamp = NanoSeconds (t);
}

void MiTag::SetTimestamp (Time time)
{
	m_timestamp = time;
}

Time MiTag::GetTimestamp (void) const
{
	return m_timestamp;
}

void MiTag::Print (std::ostream &os) const
{
	os << "t=" << m_timestamp;
}

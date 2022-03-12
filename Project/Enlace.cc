/*
Proyecto - Simulación de Redes
Fecha: 29/1/18
Alumnos: Assumpta Maria Cabral Otero
		 Luis Chacon Palos
		 Daniel Conde Ortiz
		 Miguel Ruiz Ramos
		 
Fichero: Enlace.cc
Descripcion: Este fichero contiene el código de la clase Enlace, necesaria para observar el escenario del proyecto y obtener los
			 resultados estadisticos.
*/

#include "Enlace.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/tag-buffer.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Enlace");

//Constructor de la clase enlace
Enlace::Enlace()
{
	bytes_tx = 0;
	bytes_rx = 0;
	NS_LOG_INFO("Objeto Enlace creado sin parametros");
	m_enlace_ptr = NULL;
}

//Nos sirve para añadir un Device al objeto una vez se ha creado
//Tambien programamos las trazas para cuando se envie o reciba un paquete
bool Enlace::Trazas(Ptr<PointToPointNetDevice> device_enlace)
{
	if (m_enlace_ptr == NULL)
	{
		m_enlace_ptr = device_enlace;
		m_enlace_ptr->TraceConnectWithoutContext ("MacTx", MakeCallback (&Enlace::EnviaPaquete, this));
		NS_LOG_DEBUG("Traza situada en el nodo origen, funcion trazas");
		m_enlace_ptr->TraceConnectWithoutContext ("MacRx", MakeCallback (&Enlace::RecibePaquete, this));
		NS_LOG_DEBUG("Traza en el destino, funcion trazas");
		return true;
	}
	else
	{
		NS_LOG_DEBUG("El objeto enlace ya posee un Device");
		return false;
	}	

}

//Cada vez que se envia un paquete, suma su tamaño al total
void Enlace::EnviaPaquete (Ptr<const Packet> pqt)
{
	bytes_tx += pqt->GetSize();
}

//Cada vez que se recibe un paquete, suma su tamaño al total
void Enlace::RecibePaquete (Ptr<const Packet> pqt)
{
	bytes_rx += pqt->GetSize();
}

//Devuelve el total del tamaño de paquetes enviados y recibidos
uint64_t Enlace::DevuelveBytes()
{
	return bytes_rx + bytes_tx;
}
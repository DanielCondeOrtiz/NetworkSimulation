/*
Proyecto - Simulación de Redes
Fecha: 29/1/18
Alumnos: Assumpta Maria Cabral Otero
		 Luis Chacon Palos
		 Daniel Conde Ortiz
		 Miguel Ruiz Ramos
		 
Fichero: observador.cc
Descripcion: Este fichero contiene el código de la clase Observador, necesaria para observar el escenario del proyecto y obtener los
			 resultados estadisticos. Tambien contiene el codigo de la clase MyTag para las etiquetas.
*/

#include "Observador.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/tag-buffer.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de la clase observador
//Tambien programamos las trazas
Observador::Observador (Ptr<CsmaNetDevice> device_origen, Ptr<CsmaNetDevice> device_destino, uint32_t num_origen)
{	
	NS_LOG_INFO("Observador creado");
	m_nodo_origen = num_origen;
	m_origen_ptr = device_origen;
	m_destino_ptr= device_destino;	
	primero_recibido = 0;

	m_origen_ptr->TraceConnectWithoutContext ("MacTx", MakeCallback (&Observador::EnviaPaquete, this));
	NS_LOG_DEBUG("Traza situada en el nodo origen");

	m_destino_ptr->TraceConnectWithoutContext ("MacRx", MakeCallback (&Observador::RecibePaquete, this));
	NS_LOG_DEBUG("Traza en el destino");
}

//Metodo para añadir una etiqueta a cada paquete enviado
void Observador::EnviaPaquete (Ptr<const Packet> pqt)
{
	MiTag etiqueta;
	etiqueta.SetNum(m_nodo_origen);
	pqt->AddPacketTag(etiqueta);
	NS_LOG_DEBUG("Paquete enviado con etiqueta numero: " << m_nodo_origen);
}

//Metodo para, cada vez que se reciba un paquete, comprobar si viene del nodo
//que deseamos (mediante la etiqueta) y acumular el intervalo entre cada paquete
void Observador::RecibePaquete (Ptr<const Packet> pqt)
{
	
	MiTag tagCopy;
	bool flag = false;
	flag = pqt->PeekPacketTag (tagCopy);
	if(flag == true)
	{
		if(tagCopy.GetNum() == m_nodo_origen)
		{
			NS_LOG_DEBUG("Paquete con etiqueta del nodo "<< m_nodo_origen <<" recibido");
			if(primero_recibido)
				m_intervalo.Update ((Simulator::Now() - ultimo).GetDouble());	
			else
				primero_recibido = 1;

			ultimo = Simulator::Now();
		}
	}
}

//Devuelve el maximo intervalo entre paquetes recibidos
Time Observador::GetIntervaloParada()
{
	Time intervalo(m_intervalo.Max());
	return intervalo;
}

//Clase para las etiquetas
TypeId MiTag::GetTypeId (void)
{
	static TypeId tid = TypeId ("MiTag")
	.SetParent<Tag> ()
	.AddConstructor<MiTag> ()
	.AddAttribute ("Nodo",
		"Numero del Nodo",
		EmptyAttributeValue (),
		MakeUintegerAccessor (&MiTag::GetNum),
		MakeUintegerChecker<uint32_t> ())
	;
	return tid;
}

TypeId MiTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}

uint32_t MiTag::GetSerializedSize (void) const
{
	return 4;
}

void MiTag::Serialize (TagBuffer i) const
{
	i.WriteU32 (m_num);
}

void MiTag::Deserialize (TagBuffer i)
{
	m_num = i.ReadU32 ();
}

void MiTag::SetNum (uint32_t num)
{
	m_num = num;
}

uint32_t MiTag::GetNum (void) const
{
	return m_num;
}

void MiTag::Print (std::ostream &os) const
{
	os << "Nodo=" << m_num;
}

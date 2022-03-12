/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*******************************************************************
 *******************************************************************
 *** Para poder utilizar este módulo es imprescindible convertir ***
 *** en virtual el método Receive de PointToPointNetDevice.      ***
 *** Hay que modificar:                                          ***
 ***     src/point-to-point/model/point-to-point-net-device.h    ***
 *** y en la línea 150 sustituir                                 ***
 ***     void Receive (Ptr<Packet> p);                           ***
 *** por:                                                        ***
 ***     virtual void Receive (Ptr<Packet> p);                   ***
 *******************************************************************
 *******************************************************************/

#include <ns3/core-module.h>
#include <ns3/point-to-point-remote-channel.h>
#include <ns3/mpi-interface.h>
#include <ns3/mpi-receiver.h>
#include <ns3/queue.h>
#include <ns3/ppp-header.h>
#include "Enlace.h"
#include <ns3/ipv4-header.h>
#include <algorithm>
#include "ns3/node.h"
#define PAQUETE 1
#define ACK		0

#define PENDIENTE 0
#define ENVIADO 1
#define RECIBIDO 2  //cuando se recibe un paquete o asentimiento correspondiente a un valor de la ventana

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Enlace");

NS_OBJECT_ENSURE_REGISTERED (Enlace);




///////////////////////////////////////////////////////////////
//
// Clase Enlace
//
///////////////////////////////////////////////////////////////

Enlace::Enlace ():
PointToPointNetDevice ()
{
	//Valores por defecto para las ventanas
	m_ventanatx = 1;
	m_ventanarx = 1;
}

/*
-Configura
	Asigna valores a todas las variables del enlace y crea las tablas dinamicas
*/
void Enlace::Configura (uint16_t tamano_ventanatx, uint16_t tamano_ventanarx, Time timer, Ptr<RateErrorModel> errores)
{
	m_ventanatx = tamano_ventanatx;
	m_ventanarx = tamano_ventanarx;
	m_timer = timer;
	m_receiveErrorModel=errores;

	valor_inf_tx=0; //Valor inferior de la ventana TX
	valor_inf_rx=0;	//Valor inferior de la ventana RX
	ns_envio=0;		//Numero de secuencia correspondiente al próximo envío
	t_total=0;

	NS_LOG_DEBUG("Tamano ventana tx:" << m_ventanatx);
	NS_LOG_DEBUG("Tamano ventana rx:" << m_ventanarx);
	ventanatx = (uint16_t *)calloc(m_ventanatx, sizeof(uint16_t));
	ventanarx = (uint16_t *)calloc(m_ventanarx, sizeof(uint16_t));

	//El crédito vale lo mismo que la ventana de transmisión
	credito = m_ventanatx;

	//Nos aseguramos de que ambas ventanas están inicializadas a 0
	for (uint16_t x=0; x < m_ventanarx; x++)
		ventanarx[x] = 0;
	for (uint16_t y=0; y < m_ventanatx; y++)
		ventanatx[y] = 0;

	//La cantidad de números de secuencia es igual al tamaño de la mayor de las ventanas por 2 (emisor y receptor deben tener las ventanas iguales)
	numeros_de_secuencia = std::max(m_ventanatx, m_ventanarx) * 2; //Cuandos ambas ventanas valen 1, numeros_secuencia vale 2 (0 y 1)
	temporizadores = (EventId *)calloc(numeros_de_secuencia, sizeof(EventId));

}


TypeId
Enlace::GetTypeId (void)
{
	//Hemos intentado establecer los valores de las ventanas mediante atributos, pero por algún motivo los valores no se inicializaban al valor indicado
	static TypeId tid = TypeId ("ns3::Enlace")
	.SetParent<PointToPointNetDevice> ()
	.AddConstructor<Enlace> ()
	/*.AddAttribute("VentanaRx", "Tamano de la ventana de recepcion",
					UintegerValue(1),
					MakeUintegerAccessor (&Enlace::m_ventanarx),
                   	MakeUintegerChecker<uint16_t> ())
	.AddAttribute("VentanaTx", "Tamano de la ventana de transmision",
					UintegerValue(1),
					MakeUintegerAccessor (&Enlace::m_ventanatx),
                   	MakeUintegerChecker<uint16_t> ())*/ 
	.AddTraceSource ("BitAltTx", 
		"Nuevo paquete recibido desde el nivel de red",
		MakeTraceSourceAccessor (&Enlace::m_bitAltTxTrace),
		"ns3::Packet::TracedCallback")
	;
	return tid;
}

/*
-Send
	Añade cabecera al paquete, comprueba que se pueda enviar, y programa temporizadores
*/
bool
Enlace::Send (Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo)
{

    // Procesamiento de la capa Enlace
    //¿Queda credito para enviar?
	if(credito > 0)
	{			
		NS_LOG_FUNCTION_NOARGS ();

		//Añadimos la cabecera adecuada
		EnlaceHeader cabecera;
		cabecera.SetData(ns_envio, PAQUETE);
		paquete->AddHeader(cabecera);

		m_bitAltTxTrace (paquete);

		NS_LOG_INFO("Trama con NS " << unsigned(cabecera.GetNS()) << " transmitida");

		//Enviamos paquete
		this->PointToPointNetDevice::Send (paquete, direccion, protocolo);
		credito--;

		//Actualiza los valores de las ventanas
		if(ns_envio>=valor_inf_tx)
			ventanatx[ns_envio - valor_inf_tx] = ENVIADO;
		else
			ventanatx [ns_envio + numeros_de_secuencia - valor_inf_tx] = ENVIADO;
		
		//Programamos el timer del paquete para el reenvio
		//Funciona pero no conseguimos liberar la memoria o pasa algo que da error al cabo de un rato
		//temporizadores[ns_envio] = Simulator::Schedule(Simulator::Now() + m_timer, &Enlace::Reenvia, this, paquete, direccion, protocolo);

		//Cambiamos el numero de secuencia al siguiente
		if(ns_envio<numeros_de_secuencia-1)
			ns_envio++;
		else
			ns_envio=0;
		
		
		return true;
	}
	else
		return false;
}

/*
-SendACK
	Envia un paquete de ACK con el numero de secuencia deseado
*/
void
Enlace::SendACK (Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo, uint8_t ns_recibido)
{
	NS_LOG_FUNCTION_NOARGS ();
	EnlaceHeader cabecera;

	NS_LOG_INFO("ACK "<< unsigned(ns_recibido) << " transmitido");
	
	//Añadimos cabecera de ACK al paquete
	cabecera.SetData(ns_recibido, ACK);
	paquete->AddHeader(cabecera);

	m_bitAltTxTrace (paquete);
	
	//Enviamos paquete
	this->PointToPointNetDevice::Send (paquete, direccion, protocolo);
}

/*
-Receive
	Inspecciona cabecera, comprueba que paquete está dentro de ventana de recepcion,
	cambia los valores de la tabla de ventanas y pasa paquete a nivel 3
*/
void
Enlace::Receive (Ptr<Packet> paquete)
{
	NS_LOG_FUNCTION_NOARGS ();

	//tomamos las cabeceras ip y ppp
	EnlaceHeader cabecera;
	Ipv4Header iph;
	PppHeader ppph;
	paquete->RemoveHeader(ppph);
	paquete->RemoveHeader(cabecera);

	uint8_t ns_recibido = cabecera.GetNS();
	uint16_t desplaza = 0;

	//¿Hay error al recibir?
	if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (paquete) ) 
		NS_LOG_DEBUG("Trama perdida");

	//Si no hay error
	else{

		switch(cabecera.GetTipo())
		{
			case ACK:
			NS_LOG_INFO("ACK " << unsigned(cabecera.GetNS()) << " recibido");


			//Si el ACK recibido esta dentro de la ventana de transmision desplazamos y modificamos los valores de la tabla de ventanas
			if(((ns_recibido < m_ventanatx + valor_inf_tx) && (ns_recibido >= valor_inf_tx))||((ns_recibido < valor_inf_tx)&&(ns_recibido < valor_inf_tx - numeros_de_secuencia + m_ventanatx)))
			{
				credito++;
				if (ns_recibido >= valor_inf_tx)
					ventanatx[ns_recibido - valor_inf_tx] = RECIBIDO;
				else
					ventanatx[ns_envio - valor_inf_tx] = RECIBIDO;

				desplaza = DesplazaVentanaTx();

				if (desplaza + valor_inf_tx >= numeros_de_secuencia)
				{
					valor_inf_tx = valor_inf_tx + desplaza - numeros_de_secuencia;
				}
				else
					valor_inf_tx += desplaza;

				temporizadores[ns_recibido].Cancel();

			}
			else
				NS_LOG_DEBUG("ACK recibido fuera de ventana de transmision. Borde inferior de la ventanaTx: " << unsigned(valor_inf_tx));
			break;

			case PAQUETE:
			NS_LOG_INFO("Trama con NS " << unsigned(ns_recibido) << " recibida");

			//Si el paquete recibido esta dentro de la ventana de recepcion desplazamos y modificamos los valores de la tabla de ventanas
			if(((ns_recibido < (m_ventanarx + valor_inf_rx)) && (ns_recibido >= valor_inf_rx))||((ns_recibido < valor_inf_rx)&&(ns_recibido < valor_inf_rx - numeros_de_secuencia + m_ventanarx)))
			{
				if (ns_recibido >= valor_inf_rx)
					ventanarx[ns_recibido - valor_inf_rx] = RECIBIDO;
				else
					ventanarx[ns_recibido + numeros_de_secuencia - valor_inf_rx] = RECIBIDO;

				desplaza = DesplazaVentanaRx();

				if(valor_inf_rx + desplaza >= numeros_de_secuencia)
					valor_inf_rx = valor_inf_rx + desplaza - numeros_de_secuencia;
				else
					valor_inf_rx += desplaza;	

				paquete->RemoveHeader(iph);

				//Sumamos el tamaño del paquete al total
				t_total+=paquete->GetSize();

				//Añadimos cabeceras ip y ppp al paquete
				Ptr<Packet> paquet = new Packet();
				paquete->AddHeader(iph);
				paquete->AddHeader(ppph);

			//Pasa el paquete al nivel superior
				this->PointToPointNetDevice::Receive (paquete);
				NS_LOG_INFO("Trama entregada al nivel 3");

			//Envia el ack
			SendACK(paquet, iph.GetSource(), 2048, cabecera.GetNS());	//Cambiar el 0x0800 por iph.getProtocol o ppph.getprotocol, depende de cual funcione

		}
		else
			NS_LOG_INFO("Trama con NS " << unsigned(ns_recibido) << " recibida fuera de la ventana de recepcion");
		break;
	}
}

}

/*
-ProgramaReenvia
	Programa el timer para el reenvio de un paquete
*/
void Enlace::ProgramaReenvia(Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo)
{
	NS_LOG_FUNCTION_NOARGS ();

	//Mira el numero de secuencia del paquete
	EnlaceHeader cabecera;
	paquete->PeekHeader(cabecera);
	uint8_t ns_paq = cabecera.GetNS();
	
	//Programa el reenvio para cuando venza el timer
	//Funciona pero no conseguimos liberar la memoria o pasa algo que da error al cabo de un rato
	//temporizadores[ns_paq] = Simulator::Schedule(Simulator::Now() + m_timer, &Enlace::Reenvia, this, paquete, direccion, protocolo);
}

/*
-Reenvia
	Funcion que se programa para reenviar un paquete una vez venza su temporizador
*/
void Enlace::Reenvia(Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo)
{
	NS_LOG_FUNCTION_NOARGS ();

	EnlaceHeader cabecera;
	paquete->PeekHeader(cabecera);
	NS_LOG_INFO("Trama con NS " << unsigned(cabecera.GetNS()) << " retransmitida");
	
	//Envia y vuelve a encender el timer
	this->PointToPointNetDevice::Send(paquete, direccion, protocolo);
	ProgramaReenvia(paquete,direccion,protocolo);
}

/*
-DesplazaVentanaTx
	Desplaza la tabla de ventana de transmision
*/
uint16_t Enlace::DesplazaVentanaTx()
{
	NS_LOG_FUNCTION_NOARGS ();

	uint16_t desplazamiento = 0;
	uint16_t i;
	//Desplazamos hasta que el valor de la izquierda sea 0, no recibido o no enviado
	while(ventanatx[0] == RECIBIDO)
	{

		//Esta función hace lo mismo que la operación << a los numeros binarios, pero con los valores de la tabla
		for(i=0;i<m_ventanatx; i++)
		{
			ventanatx[i]=ventanatx[i+1];	//Desplazamos los valores 1 posición a la izquierda
			ventanatx[m_ventanatx-1]=PENDIENTE;	//Introducimos un 0 por la derecha
		}
		desplazamiento++;
	}
	return desplazamiento;
}


/*
DesplazaVentanaRx
	Desplaza la tabla de ventana de recepcion
*/
uint16_t Enlace::DesplazaVentanaRx()
{
	NS_LOG_FUNCTION_NOARGS ();

	uint16_t desplazamiento = 0;
	uint16_t i;
	//Desplazamos hasta que el valor de la izquierda sea 0, no recibido o no enviado
	while(ventanarx[0] == RECIBIDO)
	{

		//Esta función hace lo mismo que la operación << a los numeros binarios, pero con los valores de la tabla
		for(i=0;i<m_ventanatx; i++)
		{
			ventanarx[i]=ventanarx[i+1];	//Desplazamos los valores 1 posición a la izquierda
			ventanarx[m_ventanarx-1]=PENDIENTE;	//Introducimos un 0 por la derecha
		}
		desplazamiento++;
	}
	return desplazamiento;
}

/*
-GetDatos
	Devuelve el total de datos transmitidos
*/
double Enlace::GetDatos(){
	return t_total;
}


///////////////////////////////////////////////////////////////
//
// Clase EnlaceHeader
//
///////////////////////////////////////////////////////////////

TypeId EnlaceHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::EnlaceHeader")
	.SetParent<Header> ()
	.AddConstructor<EnlaceHeader> ()
	;
	return tid;
}

TypeId EnlaceHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t 
EnlaceHeader::GetSerializedSize () const
{
  // two bytes of data to store
	return 2;
}
void EnlaceHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8 (m_ns);
	start.WriteU8 (m_tipo);
}
uint32_t EnlaceHeader::Deserialize (Buffer::Iterator start)
{
	m_ns = start.ReadU8 ();
	m_tipo = start.ReadU8 ();
	return 2;
}
void 
EnlaceHeader::Print (std::ostream &os) const
{
	os << "num seq = "<< m_ns << "tipo = " << m_tipo;
}

/*
-SetData
	Asigna valores a las variables de la cabecera
*/

void EnlaceHeader::SetData (uint8_t ns, uint8_t tipo)
{
	m_ns = ns;
	m_tipo = tipo;
}

/*
-GetNS
	Devuelve el numero de secuencia del paquete

*/
uint8_t EnlaceHeader::GetNS(void)
{
	return m_ns;
}

/*			
-GetTipo
	Devuelve el tipo de paquete (Normal o ACK)
*/
uint8_t EnlaceHeader::GetTipo (void)
{
	return m_tipo;
}

///////////////////////////////////////////////////////////////
//
// Clase EnlaceHelper
//
///////////////////////////////////////////////////////////////

EnlaceHelper::EnlaceHelper () :
PointToPointHelper ()
{
	m_queueFactory.SetTypeId ("ns3::DropTailQueue");
	m_deviceFactory.SetTypeId ("ns3::Enlace");
	m_channelFactory.SetTypeId ("ns3::PointToPointChannel");
	m_remoteChannelFactory.SetTypeId ("ns3::PointToPointRemoteChannel");
}

NetDeviceContainer 
EnlaceHelper::Install (NodeContainer nodos, uint16_t ventanatx, uint16_t ventanarx, Time timer, Ptr<RateErrorModel> errores)
{
	NS_ASSERT (nodos.GetN () == 2);

  // Un canal punto a punto
	Ptr<PointToPointChannel> canal = CreateObject<PointToPointChannel> ();

  // Dos nodos
	Ptr<Node> nodoTx = nodos.Get (0);
	Ptr<Node> nodoRx = nodos.Get (1);

  // Dos dispositivos de red punto a punto, ...
	NetDeviceContainer dispositivos;
	Ptr<Enlace> dispTx = CreateObject<Enlace> ();
	Ptr<Enlace> dispRx = CreateObject<Enlace> ();
	dispTx->Configura(ventanatx,ventanarx, timer, errores);
	dispRx->Configura(ventanatx,ventanarx, timer, errores);
	dispositivos.Add (dispTx);
	dispositivos.Add (dispRx);
  // Le asignamos una dirección MAC
	dispTx->SetAddress (Mac48Address::Allocate ());
	dispRx->SetAddress (Mac48Address::Allocate ());
  // Y una cola
	ObjectFactory m_queueFactory;
	m_queueFactory.SetTypeId ("ns3::DropTailQueue");
	dispTx->SetQueue (m_queueFactory.Create<Queue> ());
	dispRx->SetQueue (m_queueFactory.Create<Queue> ());

  // Asociamos los dispositivos a los nodos
	nodoTx->AddDevice (dispTx);
	nodoRx->AddDevice (dispRx);

  // Conectamos los dispositivos al canal
	dispTx->Attach (canal);
	dispRx->Attach (canal);

	return dispositivos;
}
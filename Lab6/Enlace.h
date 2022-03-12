/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/header.h>
#include <ns3/event-id.h>
#include <ns3/simulator.h>
#include "ns3/error-model.h"
using namespace ns3;

///////////////////////////////////////////////////////////////
/* Clase Enlace
Descripción: Clase Enlace que implementa a la clase PointToPointNetDevice


Métodos:	-Configura
				Asigna valores a todas las variables del enlace y crea las tablas dinamicas
			-Send
				Añade cabecera al paquete, comprueba que se pueda enviar, y programa temporizadores
			-Receive
				Inspecciona cabecera, comprueba que paquete está dentro de ventana de recepcion,
				cambia los valores de la tabla de ventanas y pasa paquete a nivel 3
			-GetDatos
				Devuelve el total de datos útiles transmitidos
			-SendACK
				Envia un paquete de ACK con el numero de secuencia deseado
			-Reenvia
				Funcion que se programa para reenviar un paquete una vez venza su temporizador
			-DesplazaVentanaTx
				Desplaza la tabla de ventana de transmision
			-DesplazaVentanaRx
				Desplaza la tabla de ventana de recepcion
			-ProgramaReenvia
				Programa el timer para el reenvio de un paquete

Atributos:	-credito
				Credito que nos queda disponible para transmitir mas paquetes	
			-temporizadores
				Tabla con los identificadores de los temporizadores
			-ventanatx
				Tabla con valores para el correcto envio de paquetes
			-ventanatx
				Tabla con valores para comprobar si paquete recibido es correcto
			-valor_inf_tx
				Valor inferior de la ventana de transmision
			-valor_inf_rx
				Valor inferior de la ventana de recepcion
			-ns_envio;
				Numero de secuencia del envio actual
			-numeros_de_secuencia;
				Total de numeros de secuencia
			-m_ventanatx
				Tamañode la ventana de transmision
			-m_ventanarx
				Tamao de la ventana de recepcion
			-m_timer
				Tiempo a programar para esperar hasta el reenvio
			-t_total
				Acumulador de tamaño de datos transmitidos
			-m_receiveErrorModel;
				Modelo de error a usar
*/				
///////////////////////////////////////////////////////////////

class Enlace : public PointToPointNetDevice
{
public:
	static TypeId GetTypeId (void);
	Enlace();
	void Configura (uint16_t tamano_ventanatx, uint16_t tamano_ventanarx, Time timer, Ptr<RateErrorModel> errores);

	virtual bool Send (Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo);
	virtual void Receive (Ptr<Packet> paquete);  
	double GetDatos();

private:
	void SendACK (Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo, uint8_t ns_recibido);
	void Reenvia (Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo);
	uint16_t DesplazaVentanaTx();
	uint16_t DesplazaVentanaRx();
	void ProgramaReenvia(Ptr<Packet> paquete, const Address & direccion, uint16_t protocolo);

  // CallBack de nuevo paquete para transmitir
	TracedCallback<Ptr<const Packet> > m_bitAltTxTrace;
	uint16_t credito;
	EventId * temporizadores;
	uint16_t * ventanarx;
	uint16_t * ventanatx;
	uint16_t valor_inf_tx; //Valor inferior de la ventana TX
	uint16_t valor_inf_rx;	//Valor inferior de la ventana RX
	uint8_t ns_envio;
	uint16_t numeros_de_secuencia;
	uint16_t m_ventanatx;
	uint16_t m_ventanarx;
	Time m_timer;
	Ptr<RateErrorModel> m_receiveErrorModel;
	double t_total;
};

///////////////////////////////////////////////////////////////
//
// Clase EnlaceHelper
//
///////////////////////////////////////////////////////////////

class EnlaceHelper: public PointToPointHelper
{
public:
	EnlaceHelper ();
	NetDeviceContainer Install(NodeContainer c, uint16_t ventanatx, uint16_t ventanarx, Time timer, Ptr<RateErrorModel> errores);
private:  
  ObjectFactory m_queueFactory;         //!< Queue Factory
  ObjectFactory m_channelFactory;       //!< Channel Factory
  ObjectFactory m_remoteChannelFactory; //!< Remote Channel Factory
  ObjectFactory m_deviceFactory;        //!< Device Factory
};


///////////////////////////////////////////////////////////////
/* Clase EnlaceHeader
Descripción: Clase para las cabeceras que se van a poner a los paquetes


Métodos:	-SetData
				Asigna valores a las variables de la cabecera
			-GetNS
				Devuelve el numero de secuencia del paquete
			-GetTipo
				Devuelve el tipo de paquete (Normal o ACK)

Atributos:	-m_ns
				Numero de secuencia del paquete
			-m_tipo
				Tipo de paquete (Normal o ACK)
*/				
///////////////////////////////////////////////////////////////

class EnlaceHeader : public Header
{
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual void Print (std::ostream &os) const;

  // Acceder a datos
	void SetData (uint8_t ns, uint8_t tipo);
	uint8_t GetNS (void);
	uint8_t GetTipo (void);
private:
	uint8_t m_ns;
	uint8_t m_tipo;
};


/*
Practica 04 - Simulación de Redes
Fecha: 19/11/17
Alumnos: Daniel Conde Ortiz
		 Miguel Ruiz Ramos

Fichero: Observador.h
Descripcion: Este fichero contiene la cabecera del fichero Observador.cc donde se declaran las clases MiTag y Observador junto con sus métodos y atributos
*/


#ifndef OBSERVADOR_H
#define OBSERVADOR_H

#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/callback.h"
#include "ns3/csma-module.h"
#include "ns3/average.h"

#include "ns3/point-to-point-module.h"
#include "ns3/simulator.h"


using namespace ns3;

/* 

Clase: MiTag
Descripción: Clase Tag para etiquetar los paquetes con el tiempo en el que fueron enviados
Métodos:	GetTimeStamp: Establece el valor de la variable Tiempo que viajara dentro de la etiqueta
			SetTimeStamp: Devuelve el parametro Time que había en la etiqueta
Atributos:
			m_timestamp: valor de la etiqueta
*/
class MiTag : public Tag {
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;

	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);


	void SetTimestamp (Time time);
	Time GetTimestamp (void) const;

	void Print (std::ostream &os) const;

private:
	Time m_timestamp;
};


/*
Clase: Observador
Descripción: Clase Observador para gestionar las trazas de los dispositivos fuentes y sumidero

Parámetros: -NetDeviceContainer * csmaDevices
				Puntero al contenedor que contiene los nodos en los que se ejecutan las aplicaciones On/off
			-Ptr<PointToPointNetDevice> device_sumidero
				Puntero al nodo donde se encuentra la aplicación sumidero
			-ApplicationContainer * sumideroapp
				Puntero al contenedor de la aplicación sumidero, necesario para detenerla cuando las aplicaciones fuente se detengan

Métodos:	-ProgramaTrazas
				Funcion que programa las trazas callback para observar los devices
			-GetRetardoMedio
				Devuelve el retardo medio calculado de todos los paquetes
			-GetPaqCorrectos
				Devuelve el valor en % de paquetes recibidos correctamente
			-Comprueba
				Comprueba periodicamente que las fuentes siguen activas para no apagar el sumidero antes de que todas las fuentes se apaguen
			-PaqueteBajo
				Metodo que se ejecuta cuando un paquete es enviado
			-PaqueteSube
				Metodo que se ejecuta cuando un paquete es recibido

Atributos:	-m_sumidero_ptr
				Puntero al device p2p en el que se encuentra el sumidero
			-m_csmaDevice
				Puntero al contenedor de devices del bus csma
			-m_sumideroapp
				Puntero al contenedor de aplicacion del sumidero
			-ultimo
				Instante en el que fue enviado el último paquete
			-m_retardos
				Variable Average para almacenar el retardo de cada paquete
			-enviados
				Número de paquetes enviados
			-recibidos
				Numero de paquetes recibidos
*/				
class Observador : public Object
{
public:
	Observador (NetDeviceContainer * csmaDevices, Ptr<PointToPointNetDevice> device_sumidero, ApplicationContainer * sumideroapp);

	void     	ProgramaTrazas ();
	double   	GetRetardoMedio ();
	double		GetPaqCorrectos ();
	void 		Comprueba();
private:  
	void     	PaqueteBaja (Ptr<const Packet> pqt);
	void     	PaqueteSube (Ptr<const Packet> pqt);

	///Atributos privados
	Ptr<PointToPointNetDevice> m_sumidero_ptr;
	NetDeviceContainer * m_csmaDevices;
	ApplicationContainer * m_sumideroapp;
	
	Time ultimo;

	Average<double> m_retardos;

	double enviados;
	double recibidos;
};

#endif

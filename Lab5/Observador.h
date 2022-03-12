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
#include <ns3/gnuplot.h>

#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/packet-sink.h"
#include "ns3/onoff-application.h"
#include "ns3/tcp-socket-base.h"

using namespace ns3;

/*
Clase: Observador
Descripción: Clase Observador para gestionar las trazas de los dispositivos fuentes y sumidero

Parámetros: -Ptr<CsmaNetDevice> csmaDevicefuente, 
				Puntero al device donde se encuentra la aplicacion fuente
			-Ptr<PointToPointNetDevice> device_sumidero
				Puntero al nodo donde se encuentra la aplicación sumidero
			-ApplicationContainer * sumideroapp
				Puntero al contenedor de la aplicación sumidero, necesario para detenerla cuando las aplicaciones fuente se detengan
			-ApplicationContainer * fuentes
				Puntero al contenedor de la aplicación fuente, necesario para obtener el socket creado
			-uint32_t flag_grafica
				Indica cuando hay que generar el dataset con los datos de la fuente

Métodos:	-ProgramaTrazas
				Funcion que programa las trazas callback para observar los devices
			-TrazaSocket
				Devuelve el retardo medio calculado de todos los paquetes
			-GetDatos
				Devuelve el valor en % de paquetes recibidos correctamente
			-GetOctetosRecibidos
				Devuelve el numero de octetos recibidos
			-Comprueba
				Comprueba periodicamente que las fuentes siguen activas para no apagar el sumidero antes de que todas las fuentes se apaguen
			-PaqueteSube
				Metodo que se ejecuta cuando un paquete es recibido
			-GetCongestion
				Introduce los datos de la ventana de congestion en el dataset para posteriormente representar las graficas

Atributos:	-m_sumidero_ptr
				Puntero al device p2p en el que se encuentra el sumidero
			-m_csmaDeviceFuente
				Puntero al device de la fuente
			-m_sumideroapp
				Puntero al contenedor de aplicacion del sumidero
			-m_fuentes
				Puntero al contenedor de aplicacion fuente
			-ultimo
				Instante en el que fue enviado el último paquete
			-sockeFuente
				Puntero al socket Tcp
			-CongWin
				Puntero al dataset de la ventana de congestion
			-m_flag_grafica
				Variable flag que indica cuando hay que crear la gráfica con la ventana de congestion
			-octetosRx
				Variable que almacena los octetos recibidos correctamente en la conexion
			-recibidos
				Número de paquetes recibidos
			-m_tstop
				Valor de tstop de la fuente
*/				
class Observador
{
public:
	Observador (Ptr<CsmaNetDevice> csmaDevicefuente, Ptr<PointToPointNetDevice> device_sumidero, ApplicationContainer * sumideroapp, ApplicationContainer * fuentes, uint32_t flag_grafica, Time tstop);

	Gnuplot2dDataset * GetDatos();
	double 		GetOctetosRecibidos();

private:  
	void     	PaqueteSube (Ptr<const Packet> pqt);
	void 		GetCongestion(uint32_t old, uint32_t newo);
	void 		Comprueba();
	void     	ProgramaTrazas ();
	void 		TrazaSocket ();


	///Atributos privados
	Ptr<PointToPointNetDevice> m_sumidero_ptr;
	Ptr<CsmaNetDevice> m_csmaDevicefuente;
	ApplicationContainer * m_sumideroapp;
	ApplicationContainer * m_fuentes;

	Ptr<TcpSocketBase> socketFuente;

	Gnuplot2dDataset * CongWin;
	uint32_t m_flag_grafica;
	double octetosRx;
	uint32_t recibidos;
	Time m_tstop;
	Time ultimo;
};

#endif

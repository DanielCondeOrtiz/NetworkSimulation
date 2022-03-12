/*
Proyecto - Simulación de Redes
Fecha: 29/1/18
Alumnos: Assumpta Maria Cabral Otero
		 Luis Chacon Palos
		 Daniel Conde Ortiz
		 Miguel Ruiz Ramos
		 
Fichero: Enlace.h
Descripcion: Este fichero contiene la cabecera del fichero Enlace.cc donde se declara la clase Enlace
*/


#ifndef ENLACE_H
#define ENLACE_H

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


class Enlace : public Object
{
public:
	Enlace(void);
	uint64_t DevuelveBytes(void);
	bool Trazas (Ptr<PointToPointNetDevice> device_enlace);
private:  
	void    EnviaPaquete (Ptr<const Packet> pqt);
	void    RecibePaquete (Ptr<const Packet> pqt);

	///Atributos privados
	Ptr<PointToPointNetDevice> m_enlace_ptr; //Device
	
	uint64_t bytes_tx; //Total de bytes transmitidos
	uint64_t bytes_rx; //Total de bytes recibidos
};

#endif
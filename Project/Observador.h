/*
Proyecto - Simulación de Redes
Fecha: 29/1/18
Alumnos: Assumpta Maria Cabral Otero
		 Luis Chacon Palos
		 Daniel Conde Ortiz
		 Miguel Ruiz Ramos
		 
Fichero: observador.h
Descripcion: Este fichero contiene la cabecera del fichero Observador.cc donde se declara la clase Observador
			 Tambien contiene la declaracion de la clase MyTag
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


class Observador : public Object{
public:
	Observador (Ptr<CsmaNetDevice> origen, Ptr<CsmaNetDevice> destino, uint32_t num_origen);
	Time	GetIntervaloParada(void);

private:  
	void    EnviaPaquete (Ptr<const Packet> pqt);
	void    RecibePaquete (Ptr<const Packet> pqt);

	///Atributos privados
	Ptr<CsmaNetDevice> m_origen_ptr; //Nodo origen
	Ptr<CsmaNetDevice> m_destino_ptr; //Nodo destino
	
	uint32_t primero_recibido; //Flag
	uint32_t m_nodo_origen; //Identificador del nodo origen
	Average<double> m_intervalo; //Para ir guardando cada intervalo
	Time ultimo;
};


class MiTag : public Tag {
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;

	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);


	void SetNum (uint32_t num);
	uint32_t GetNum (void) const;

	void Print (std::ostream &os) const;

private:
	uint8_t m_num;
};

#endif
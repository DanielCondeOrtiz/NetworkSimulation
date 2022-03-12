#ifndef OBSERVADOR_H
#define OBSERVADOR_H

#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/callback.h"
#include "ns3/csma-module.h"
#include "ns3/average.h"

using namespace ns3;

class Observador : public Object
{
public:
  Observador (Ptr<CsmaNetDevice> device_ptr);

  double   	GetIntentosMedio ();
  void     	ProgramaTrazas ();
  double   	GetIntentosCorrectos();
  double   	GetRetardoMedio ();
  double 	GetPaqPerdidos ();
private:  
  void     	IntentoEnvio (Ptr<const Packet> pqt);
  void     	PaqueteEntregado (Ptr<const Packet> pqt);
  void     	PaqueteDescartado (Ptr<const Packet> pqt);
  void     	PaqueteBaja (Ptr<const Packet> pqt);
  void     	PaqueteSube (Ptr<const Packet> pqt);


  // Dispositivo al que está asociado este observador
  Ptr<CsmaNetDevice> m_miDevice_ptr;
  // Numero de intentos de Transmision para enviar un paquete
  double m_numIntentosTx;
  // Numero de paquetes perdidos
  double m_numPaqPerdidos;
  // Numero de intentos de transmision de los paquetes enviados
  Average<double> m_numIntentos;
  // Tiempo en recibir eco
  Average<double> m_retardos;
  // Instante de generación de paquete de solicitud de eco
  Time     m_instanteRxAp;
};
#endif

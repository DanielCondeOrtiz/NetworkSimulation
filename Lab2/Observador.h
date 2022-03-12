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
       
  double   GetRetardoMedio ();
  double   GetPaquetesTx ();
  double   GetPaquetesErroneos ();
  double   GetEcosRx ();
  void     ProgramaTrazas ();
  uint64_t GetNumTramas();

 private:  
  void     TiraTramaTx (Ptr<const Packet> pqt);
  void     TiraTramaRx (Ptr<const Packet> pqt);
  void     PaqueteEntregado (Ptr<const Packet> pqt);
  void     PaqueteRecibido (Ptr<const Packet> pqt);

  // Dispositivo al que está asociado este observador
  Ptr<CsmaNetDevice> m_miDevice_ptr;
  // Estadística de los retardos de eco
  Average<double> m_retardos;
  // Instante de generación de paquete de solicitud de eco
  Time     m_instanteRxAp;
  // Número de tramas transmitidas desde la aplicación
  uint64_t m_numTramasTx;
  // Número de tramas tiradas, que no se llegan a transmitir
  uint32_t m_tiradastx;
  uint32_t m_tiradasrx;
};
#endif

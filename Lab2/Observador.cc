#include "Observador.h"

using namespace ns3;



Observador::Observador (Ptr<CsmaNetDevice> device_ptr)
{
  m_numTramasTx=0;
  m_miDevice_ptr=device_ptr;
  m_tiradastx=0;
  m_tiradasrx=0;
  ProgramaTrazas ();
}

void
Observador::ProgramaTrazas ()
{
   m_miDevice_ptr->TraceConnectWithoutContext ("PhyTxDrop", MakeCallback (&Observador::TiraTramaTx, this));
   m_miDevice_ptr->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&Observador::TiraTramaRx, this));
   m_miDevice_ptr->TraceConnectWithoutContext ("MacTx", MakeCallback (&Observador::PaqueteEntregado, this));
   m_miDevice_ptr->TraceConnectWithoutContext ("MacRx", MakeCallback (&Observador::PaqueteRecibido, this));
}

void 
Observador::TiraTramaTx (Ptr<const Packet>pqt_ptr)
{
  m_tiradastx++;
}

void 
Observador::TiraTramaRx (Ptr<const Packet>pqt_ptr)
{
  m_tiradasrx++;
}

void
Observador::PaqueteEntregado (Ptr<const Packet> pqt)
{
  m_instanteRxAp=Simulator::Now ();
  m_numTramasTx++;
}

void
Observador::PaqueteRecibido (Ptr<const Packet> pqt)
{
  Time retardo=Simulator::Now ()-m_instanteRxAp;
  if (retardo > 0)
    m_retardos.Update (retardo.GetDouble ());
}

uint64_t
Observador::GetNumTramas ()
{
  return m_numTramasTx;
}

double Observador::GetRetardoMedio ()
{
  return m_retardos.Mean ();
}

double Observador::GetEcosRx ()
{
  return m_retardos.Count ();
}

double Observador::GetPaquetesTx()
{
   return m_numTramasTx;
}

double Observador::GetPaquetesErroneos()
{
   return m_tiradasrx;
}
		

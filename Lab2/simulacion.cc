/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "Observador.h"

using namespace ns3;
#define NCSMA     2
#define RETARDO   "5us"
#define TASA      "100Mb/s"
#define TAMANO    1204
#define INTERVALO "500ms"
#define STOP      "3s"
#define RUN       "1s"
#define ERROR     0



int
main (int argc, char *argv[])
{
   GlobalValue::Bind ("ChecksumEnabled", BooleanValue(true));
   uint32_t    nCsma = NCSMA;	 
   Time        delay (RETARDO);
   DataRate    rate (TASA);
   uint32_t    packetSize = TAMANO;
   Time        tstop (STOP);
   Time        tstart (RUN);
   std::string interval (INTERVALO);
   double      perror = ERROR;

   // Debemos garantizar qu hay al menos un cliente y el servidor.
   if (nCsma <= 1)
     nCsma = 2;

   // Aquí empieza el escenario
   NodeContainer csmaNodes;
   csmaNodes.Create (nCsma);
   CsmaHelper csma;
   csma.SetChannelAttribute ("DataRate", DataRateValue(rate));
   csma.SetChannelAttribute ("Delay", TimeValue (delay));
   Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
   errores->SetUnit (RateErrorModel::ERROR_UNIT_BIT);
   errores->SetRate (perror);
   csma.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (errores));
 
   NetDeviceContainer csmaDevices;
   csmaDevices = csma.Install (csmaNodes);

   // Instalamos la pila TCP/IP en todos los nodos
   InternetStackHelper stack;
   stack.Install (csmaNodes);
   // Y les asignamos direcciones
   Ipv4AddressHelper address;
   address.SetBase ("10.1.2.0", "255.255.255.0");
   Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);
   // Cálculo de rutas
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
   //Configuramos el tráfico
   /////////// Instalación de las aplicaciones
   // Servidor en el último nodo
   UdpEchoServerHelper echoServer (9);
   ApplicationContainer serverApp = echoServer.Install (csmaNodes.Get (nCsma - 1));
   serverApp.Start (Seconds(0.0));
   serverApp.Stop (tstop);
   // Clientes: en todos los nodos menos en el último
   UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma - 1), 9);
   echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
   echoClient.SetAttribute ("Interval", TimeValue (Time (interval)));
   echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
   NodeContainer clientes;
   for (uint32_t i = 0; i < nCsma - 1; i++)
     clientes.Add (csmaNodes.Get (i));
   ApplicationContainer clientApps = echoClient.Install (clientes);
   clientApps.Start (tstart);
   clientApps.Stop (tstop);
	
   //Aquí empieza la parte de análisis
   // Paquetes desde el punto de vista del servidor
   csma.EnablePcap ("p02_servidor", csmaDevices.Get (nCsma - 1), true);
   // Paquetes desde el punto de vista del cliente
   csma.EnablePcap ("p02_cliente", csmaDevices.Get (0), true);

   Observador   servidor (csmaDevices.Get (nCsma-1)->GetObject<CsmaNetDevice> ());
   Observador * observadores[nCsma-1];
   for (uint32_t i = 0; i < nCsma-1 ; i++)
     observadores[i] = new Observador (csmaDevices.Get (i)->GetObject<CsmaNetDevice> ());
 

   // Lanzamos la simulación
   Simulator::Run ();
   // Finalizamos el pro
   Simulator::Destroy ();

   return 0;
}

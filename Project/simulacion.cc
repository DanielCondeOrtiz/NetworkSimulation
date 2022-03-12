/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Proyecto - Simulación de Redes
Fecha: 29/01/18
Alumnos: Assumpta Cabral Otero
		 Luis Chacón Palos
		 Daniel Conde Ortiz
		 Miguel Ruiz Ramos

Fichero: simulacion.cc
Descripcion: Este fichero contiene la funcion main y la funcion escenario, encargadas de configurar el sistema, generar las graficas y ejecutar la simulacion
*/

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include <ns3/gnuplot.h>
#include <sstream>

#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ethernet-header.h"
#include "ns3/error-model.h"

#include "Observador.h"
#include "Enlace.h"

using namespace ns3;
#define RETARDOB   "6560ns"
#define RETARDOE   "2ms"
#define TASAB      "1Mb/s"
#define TASAE      "1Mb/s"
#define TASAFUENTE "100Kb/s"
#define TAMANO     200
#define TON			"1s"
#define TOFF		"0s"
#define STOP      "25s"
#define START       "7s"
#define TSTUDENT  1.345
#define MUESTRAS 15
#define ERROR 0.0001
#define MAXDELAY 2000

NS_LOG_COMPONENT_DEFINE ("trabajo");

//Estructura de datos en la que guardaremos los datos de cada simulacion
struct Datos{
	double bytes_enlace[6];
	double tiempo_parada;
};

void escenario (Time delayb, Time delaye, DataRate rateb, DataRate ratee, DataRate ratef, uint32_t packetSize, Time tstop, Time tstart, Time ton, 
	Time toff, Time unsolicitedUpdate, Time timeoutDelay, uint16_t version, uint16_t medirEnl, Datos * resultados);


int main (int argc, char *argv[])
{
	//Se establece la resolucion en nanosegundos
	Time::SetResolution (Time::NS);

	GlobalValue::Bind ("ChecksumEnabled", BooleanValue(true));
	
	//Se establecen los valores por defecto para las variables	 
	
	Time        delayb (RETARDOB);
	Time        delaye (RETARDOE);
	DataRate    rateb (TASAB);
	DataRate    ratee (TASAE);
	DataRate    ratef (TASAFUENTE);
	uint32_t    packetSize = TAMANO;
	Time        tstop (STOP);
	Time        tstart (START);
	Time		ton (TON);
	Time 		toff (TOFF);

	Datos resultados;

	//Para configurar los valores de las variables mediante linea de comandos
	CommandLine cmd;

	cmd.AddValue ("ton", "tiempo del periodo de las fuentes en on", ton);
	cmd.AddValue ("toff", "tiempo del periodo de las fuentes en off", toff);
	cmd.AddValue ("tamPqt", "tamano de las SDU de aplicacion de la fuente", packetSize);
	cmd.AddValue ("tasaEnvio", "tasa de bits durante el periodo en on", ratef);

	cmd.AddValue ("retardoBus", "retardo de propagación en el bus", delayb);
	cmd.AddValue ("capacidadBus", "capacidad del bus", rateb);
	cmd.AddValue ("retardoEn", "retardo de propagación en el enlace P2P", delaye);
	cmd.AddValue ("capacidadEn", "capacidad del enlace P2P", ratee);

	cmd.AddValue ("tstart", "instante de simulacion de inicio de las aplicaciones", tstart);
	cmd.AddValue ("tstop", "intante de simulacion de fin de aplicaciones clientel", tstop);

	cmd.Parse (argc, argv);


	//Para simular ambar versiones del escenario
	for(int version = 0; version < 2; version ++){
		NS_LOG_INFO("\nComienzan las simulaciones para el escenario de la version: " << version +1);

		//Para añadir informacion sobre los parametros variables a la grafica
		std::ostringstream strs1;
		std::string sratef;
		std::string spacket;
		std::string sversion;
		std::string sdelay;
		std::string sratee;

		strs1 << ratef.GetBitRate()/1e3;
		sratef = strs1.str();
		strs1.str("");

		strs1 << packetSize;
		spacket = strs1.str();
		strs1.str("");

		strs1 << MAXDELAY;
		sdelay = strs1.str();
		strs1.str("");

		strs1 << ratee.GetBitRate()/1e6;
		sratee = strs1.str();
		strs1.str("");
		
		
		if(version==0)
			strs1 << "1 (3 routers)";
		else
			strs1 << "2 (4 routers)";

		sversion = strs1.str();
		strs1.str("");


		//Comenzamos con las graficas
		Gnuplot GraficaCorte;
		GraficaCorte.SetLegend ("Tiempo de UnsolicitedRoutingUpdate (s)", "Maximo tiempo sin recibir (s)");
		GraficaCorte.SetTitle ("Tiempo maximo sin recibir paquetes variando los siguientes parametros de RIP:\\nUnsolicitedRoutingUpdate y TimeoutDelay\\nVersion del escenario: "
			+ sversion + "\\nTasa de las fuentes: " + sratef + "Kb/s; Tamanio de los paquetes: " + spacket + " (bytes)");

		Gnuplot GraficaEnlaces;
		GraficaEnlaces.SetLegend ("Tiempo de UnsolicitedRoutingUpdate (s)", "Porcentaje de ocupacion del enlace (%)");
		GraficaEnlaces.SetTitle ("Ocupacion de los enlaces segun varia el parametro de RIP:\\nUnsolicitedRoutingUpdate\\nVersion del escenario: "
			+ sversion + "\\nTasa de las fuentes: " + sratef + "Kb/s; Tamanio de los paquetes: " + spacket + " (bytes)\\nCapacidad del enlace: " + sratee + "Mb/s; TimeoutDelay: " + sdelay + "ms");

		//Creamos las lineas para los datos de los enlaces
		Gnuplot2dDataset datos_enlaces[2+3*version];

		for(uint16_t i = 0; i < 2+3*version; i++){
			datos_enlaces[i].SetStyle (Gnuplot2dDataset::LINES_POINTS);
			datos_enlaces[i].SetErrorBars (Gnuplot2dDataset::Y);
		}
		
		datos_enlaces[0].SetTitle("Enlace R3-R4");
		datos_enlaces[1].SetTitle("Enlace R3-R5");

		if(version==1){
			datos_enlaces[2].SetTitle("Enlace R3-R6");
			datos_enlaces[3].SetTitle("Enlace R4-R6");
			datos_enlaces[4].SetTitle("Enlace R5-R6");
		}

		uint16_t medirEnl=0;

		//Bucle para cada linea de tiempos (graficaCorte), variando el tiempo de timeout de las rutas
		for(Time timeoutDelay = MilliSeconds(500); timeoutDelay <= MilliSeconds(MAXDELAY); timeoutDelay+=MilliSeconds(500))
		{
			//Solo medimos la ocupacion de los enlaces en las simulaciones con el maximo timeout
			if(timeoutDelay.GetMilliSeconds() == MAXDELAY)
				medirEnl=1;

			NS_LOG_INFO("\n-Comienzan las simulaciones para la curva con timeoutDelay " << timeoutDelay.GetSeconds() << "s");

			std::ostringstream strs2;
			std::string str2;

			strs2 << timeoutDelay.GetSeconds();
			str2 = strs2.str();

			//Creamos la linea
			Gnuplot2dDataset datos_timeout("TimeoutDelay: " + str2 + "s");

			datos_timeout.SetStyle (Gnuplot2dDataset::LINES_POINTS);
			datos_timeout.SetErrorBars (Gnuplot2dDataset::Y);


			//Bucle para cada tiempo de unsolicitedUpdate
			for (Time unsolicitedUpdate = MilliSeconds(100); unsolicitedUpdate <= MilliSeconds(1000); unsolicitedUpdate += MilliSeconds(150))
			{
				if(unsolicitedUpdate <= timeoutDelay)
				{
					//Acumuladores para las lineas
					Average<double> acumTiempo;
					Average<uint64_t> acumEnlace[5];

					NS_LOG_INFO("\n--Comienzan las simulaciones para UnsolicitedRoutingUpdate: " << unsolicitedUpdate.GetSeconds() << "s");

					//Bucle para simular MUESTRAS veces para el mismo tiempo de unsolicitedUpdate
					for (uint32_t sim = 0; sim < MUESTRAS; sim++)
					{
						NS_LOG_DEBUG("\nSimulacion: " << sim +1);

						//Ponemos los valores de la estructura de nuevo a 0
						resultados.tiempo_parada = 0;
						for (uint16_t z = 0; z < 6; z++)
							resultados.bytes_enlace[z] = 0;
						

						escenario(delayb, delaye, rateb, ratee, ratef, packetSize, tstop, tstart, ton, toff, unsolicitedUpdate, timeoutDelay, version, medirEnl, &resultados);


						//Almacenamos los valores devuelto por la simulacion en acumuladores
						acumTiempo.Update(resultados.tiempo_parada);

						//La simulacion nos devuelve el total de bytes transmitidos en cada enlace
						//Nosotros calculamos el porcentaje de ocupacion en cada enlace
						for(uint16_t i=0;i < 2+3*version && medirEnl == 1; i++){
							acumEnlace[i].Update((800* resultados.bytes_enlace[i]) / (double(ratee.GetBitRate()) * (tstop.GetSeconds() - tstart.GetSeconds())));
						}
					}


					//Variables de error y añadimos los datos a las lineas
					double error = TSTUDENT * sqrt (acumTiempo.Var () / acumTiempo.Count ());
					datos_timeout.Add (unsolicitedUpdate.GetSeconds(), acumTiempo.Mean(), error);


					double errorEnl[5];

					for(uint16_t i=0;i < 2+3*version && medirEnl==1; i++){
						errorEnl[i] = TSTUDENT * sqrt (acumEnlace[i].Var () / acumEnlace[i].Count ());
						datos_enlaces[i].Add (unsolicitedUpdate.GetSeconds(), acumEnlace[i].Mean(), errorEnl[i]);
					}
				}
			}

			// Añadimos las lineas a la grafica
			GraficaCorte.AddDataset (datos_timeout);

			for(uint16_t i=0;i < 2+3*version && medirEnl==1; i++)
				GraficaEnlaces.AddDataset (datos_enlaces[i]);
			
		}

	//Generamos los fichero con las graficas
	//Dependiendo de la version del escenario se generará un fichero u otro
		if(version==0){
			std::ofstream fichero1 ("GraficaTiempo1.plt");
			GraficaCorte.GenerateOutput (fichero1);
			fichero1 << "pause -1" << std::endl;
			fichero1.close ();

			std::ofstream fichero2 ("GraficaEnlaces1.plt");
			GraficaEnlaces.GenerateOutput (fichero2);
			fichero2 << "pause -1" << std::endl;
			fichero2.close ();
		}
		else{
			std::ofstream fichero1 ("GraficaTiempo2.plt");
			GraficaCorte.GenerateOutput (fichero1);
			fichero1 << "pause -1" << std::endl;
			fichero1.close ();

			std::ofstream fichero2 ("GraficaEnlaces2.plt");
			GraficaEnlaces.GenerateOutput (fichero2);
			fichero2 << "pause -1" << std::endl;
			fichero2.close ();
		}
	}

	return 0;
}

//Funcion encargada de configurar el escenario y ejecutar la simulación.
void escenario (Time delayb, Time delaye, DataRate rateb, DataRate ratee, DataRate ratef, uint32_t packetSize, Time tstop, Time tstart, Time ton, 
	Time toff, Time unsolicitedUpdate, Time timeoutDelay, uint16_t version, uint16_t medirEnl, Datos * resultados)
{

	//Creamos todos los nodos
	NodeContainer nodos;
	nodos.Create (6 + version);

	//Contenedores de nodos para cada oficina (nos serán útiles más adelante)
	NodeContainer n_ofi1 = NodeContainer(nodos.Get(0),nodos.Get(3));
	NodeContainer n_ofi2 = NodeContainer(nodos.Get(1),nodos.Get(4));
	NodeContainer n_ofi3 = NodeContainer(nodos.Get(2),nodos.Get(5));

	//Contenedores de nodos para cada enlace entre routers
	NodeContainer r3r4 = NodeContainer(nodos.Get(3), nodos.Get(4));
	NodeContainer r3r5 = NodeContainer(nodos.Get(3), nodos.Get(5));
	NodeContainer r4r5 = NodeContainer(nodos.Get(4), nodos.Get(5));

	//Contenedores de nodos para pc's y routers en general
	NodeContainer pcs = NodeContainer(nodos.Get(0),nodos.Get(1),nodos.Get(2));
	NodeContainer routers = NodeContainer(nodos.Get(3), nodos.Get(4), nodos.Get(5));


	//Para la version 2 (1 en nuestro código), necesitaremos 3 contenedores de nodos más
	NodeContainer	r3r6;
	NodeContainer	r4r6;
	NodeContainer	r5r6;

	if(version==1){
		routers.Add(NodeContainer(nodos.Get(6)));

		r3r6 = NodeContainer(nodos.Get(3), nodos.Get(6));
		r4r6 = NodeContainer(nodos.Get(4), nodos.Get(6));
		r5r6 = NodeContainer(nodos.Get(5), nodos.Get(6));
	}


	//Asignamos valores de atributos a canales
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", DataRateValue(rateb));
	csma.SetChannelAttribute ("Delay", TimeValue (delayb));

	//Hacemos que haya un poco de error en los enlaces CSMA
	Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
	errores->SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
	errores->SetRate (ERROR);
	csma.SetDeviceAttribute("ReceiveErrorModel", PointerValue(errores));

	//En las oficinas habrá CSMA
	NetDeviceContainer d_ofi1 = csma.Install (n_ofi1);
	NetDeviceContainer d_ofi2 = csma.Install (n_ofi2);
	NetDeviceContainer d_ofi3 = csma.Install (n_ofi3);

	//Creamos el helper para PointToPoint
	PointToPointHelper p2p;
	p2p.SetChannelAttribute("Delay",TimeValue (delaye));
	p2p.SetDeviceAttribute("DataRate", DataRateValue(ratee));

	//Igual que en CSMA, hacemos que haya errores
	p2p.SetDeviceAttribute("ReceiveErrorModel", PointerValue(errores));

	//En los enlaces entre routers habrá PointToPoint
	NetDeviceContainer d_r3r4 = p2p.Install(r3r4); 
	NetDeviceContainer d_r3r5 = p2p.Install(r3r5); 
	NetDeviceContainer d_r4r5 = p2p.Install(r4r5); 

	NetDeviceContainer	d_r3r6;
	NetDeviceContainer	d_r4r6;
	NetDeviceContainer	d_r5r6;
	//De nuevo, para la versión 2 del escenario
	if(version==1){

		d_r3r6 = p2p.Install(r3r6); 
		d_r4r6 = p2p.Install(r4r6); 
		d_r5r6 = p2p.Install(r5r6); 
	}

	// Configuramos el protocolo RIP
	RipHelper ripRouting;

	ripRouting.Set("UnsolicitedRoutingUpdate", TimeValue(unsolicitedUpdate));
	ripRouting.Set("TimeoutDelay", TimeValue(timeoutDelay));

	//RIP no se ejecuta en las interfaces de los routers que están en las oficinas
	ripRouting.ExcludeInterface (nodos.Get(3), 1);
	ripRouting.ExcludeInterface (nodos.Get(4), 1);
	ripRouting.ExcludeInterface (nodos.Get(5), 1);


	//Instalamos RIP en los routers
	Ipv4ListRoutingHelper listRH;
	listRH.Add (ripRouting, 0);

	InternetStackHelper internet;
	internet.SetIpv6StackInstall (false);
	internet.SetRoutingHelper (listRH);
	internet.Install (routers);

	//Instalamos IP en los PC's
	InternetStackHelper stack;
	stack.Install (pcs);
	
	// Asignamos direcciones a cada enlace
	Ipv4AddressHelper dir_ip;
	dir_ip.SetBase ("10.0.10.0", "255.255.255.0");
	Ipv4InterfaceContainer ip_ofi1 = dir_ip.Assign (d_ofi1);

	dir_ip.SetBase ("10.0.20.0", "255.255.255.0");
	Ipv4InterfaceContainer ip_ofi2 = dir_ip.Assign (d_ofi2);
	
	dir_ip.SetBase ("10.0.30.0", "255.255.255.0");
	Ipv4InterfaceContainer ip_ofi3 = dir_ip.Assign (d_ofi3);

	dir_ip.SetBase ("10.0.34.0", "255.255.255.0");
	dir_ip.Assign (d_r3r4);

	dir_ip.SetBase ("10.0.35.0", "255.255.255.0");
	dir_ip.Assign (d_r3r5);

	dir_ip.SetBase ("10.0.45.0", "255.255.255.0");
	dir_ip.Assign (d_r4r5);

	//En el caso que que tengamos un router más
	if(version==1){
		dir_ip.SetBase ("10.0.36.0", "255.255.255.0");
		dir_ip.Assign (d_r3r6);

		dir_ip.SetBase ("10.0.46.0", "255.255.255.0");
		dir_ip.Assign (d_r4r6);

		dir_ip.SetBase ("10.0.56.0", "255.255.255.0");
		dir_ip.Assign (d_r5r6);
	}


	//Para los PC's de las oficinas, configuramos como ruta por defecto los rouers de las mismas
	Ptr<Ipv4StaticRouting> staticRouting;
	staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (n_ofi1.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
	staticRouting->SetDefaultRoute (ip_ofi1.GetAddress(1), 1 );

	staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (n_ofi2.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
	staticRouting->SetDefaultRoute (ip_ofi2.GetAddress(1), 1 );

	staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (n_ofi3.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
	staticRouting->SetDefaultRoute (ip_ofi3.GetAddress(1), 1 );


	NS_LOG_DEBUG("Creando aplicaciones");

	// Configuramos el tráfico	
	// Instalación de las aplicaciones

	//Usamos este puerto porque es uno de los que usa Skype
	uint16_t sinkPort = 3478;

	//Helper para el sumidero del PC de la oficina 1
	PacketSinkHelper sumidero ("ns3::UdpSocketFactory",InetSocketAddress(ip_ofi1.GetAddress(0),sinkPort));

	//Helper para los PC's que envien paquetes a la oficina 3
	//Nos servirá para configurar los valores globales de la aplicación y luego
	//simplemente cambiaremos la IP destino
	OnOffHelper fuente("ns3::UdpSocketFactory",InetSocketAddress(ip_ofi3.GetAddress(0),sinkPort));

	//Obtenemos los valores de ton y toff que van a tener las fuentes
	std::ostringstream strs1;
	std::ostringstream strs2;
	std::string str;

	strs1 << ton.GetSeconds();
	str = strs1.str();

	std::string ontime = "ns3::ConstantRandomVariable[Constant="+ str +"]";
	
	strs2 << toff.GetSeconds();
	str = strs2.str();

	std::string offtime = "ns3::ConstantRandomVariable[Constant="+ str +"]";

	//Se los asignamos, junto a otros atributos
	fuente.SetAttribute("OnTime", StringValue(ontime));
	fuente.SetAttribute("OffTime", StringValue(offtime));
	fuente.SetAttribute("DataRate", DataRateValue(ratef));
	fuente.SetAttribute("PacketSize", UintegerValue(packetSize));


	//Asignamos las fuentes a los PC's y los tiempos de start y stop
	ApplicationContainer fuentesApp = fuente.Install(n_ofi1.Get(0)); //Ofi1 a Ofi3
	fuentesApp.Add(fuente.Install(n_ofi2.Get(0)));					 //Ofi2 a Ofi3

	fuente.SetAttribute("Remote", AddressValue(InetSocketAddress(ip_ofi2.GetAddress(0),sinkPort)));
	fuentesApp.Add(fuente.Install(n_ofi1.Get(0)));					 //Ofi1 a Ofi2
	fuentesApp.Add(fuente.Install(n_ofi3.Get(0)));					 //Ofi3 a Ofi2

	fuente.SetAttribute("Remote", AddressValue(InetSocketAddress(ip_ofi1.GetAddress(0),sinkPort)));
	fuentesApp.Add(fuente.Install(n_ofi2.Get(0)));					 //Ofi2 a Ofi1
	fuentesApp.Add(fuente.Install(n_ofi3.Get(0)));					 //Ofi3 a Ofi1

	fuentesApp.Start(tstart);
	fuentesApp.Stop(tstop);

	//Asignamos los sumideros a los PC's y los tiempos de start y stop

	ApplicationContainer sumideroApp = sumidero.Install(n_ofi1.Get(0));	//Sumidero Ofi1

	sumidero.SetAttribute("Local",AddressValue(InetSocketAddress(ip_ofi2.GetAddress(0),sinkPort)));
	sumideroApp.Add(sumidero.Install(n_ofi2.Get(0)));					//Sumidero Ofi2
	
	sumidero.SetAttribute("Local",AddressValue(InetSocketAddress(ip_ofi3.GetAddress(0),sinkPort)));
	sumideroApp.Add(sumidero.Install(n_ofi3.Get(0)));					//Sumidero Ofi3
	
	sumideroApp.Start(tstart);
	sumideroApp.Stop(tstop);


	NS_LOG_DEBUG("Aplicaciones configuradas");

	//Creamos el observador (para calcular el maximo de tiempo sin recibir paquetes)
	Observador obs (d_ofi2.Get (0)->GetObject<CsmaNetDevice> (), d_ofi3.Get (0)->GetObject<CsmaNetDevice> (), 2);
	
	//Creamos objetos Enlace para calcular el total de bits transmitidos (y recibidos) en cada enlace
	Enlace enl1;
	Enlace enl2;
	Enlace enl4;
	Enlace enl5;
	Enlace enl6;

	if(medirEnl==1){
		NS_LOG_DEBUG("Midiendo enlaces");
		enl1.Trazas(d_r3r4.Get(0)->GetObject<PointToPointNetDevice> ());
		enl2.Trazas(d_r3r5.Get(0)->GetObject<PointToPointNetDevice> ());

		if (version == 1)
		{
			enl4.Trazas(d_r3r6.Get(0)->GetObject<PointToPointNetDevice> ());
			enl5.Trazas(d_r4r6.Get(0)->GetObject<PointToPointNetDevice> ());
			enl6.Trazas(d_r5r6.Get(0)->GetObject<PointToPointNetDevice> ());
		}

	}
	//Programamos un corte en el enlace que une R4 con R5 para que se de a los 15s de haber empezado la simulacion
	Ptr<Node> n1 = r4r5.Get (1);
	Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4> ();
	uint32_t ipv4ifIndex1 = 3;
	
	Ptr<NormalRandomVariable> caida = CreateObject<NormalRandomVariable> ();
	caida->SetAttribute("Mean", DoubleValue(15));
	caida->SetAttribute("Variance",DoubleValue(1));
	double t_caida = caida->GetValue();
	NS_LOG_DEBUG("El enlace caera en el instante: " << Seconds(t_caida) << "s");
	Simulator::Schedule (Seconds (t_caida),&Ipv4::SetDown,ipv41, ipv4ifIndex1);

	//Las aplicaciones pararán cuando están configuradas, pero el protocolo RIP no lo hará
	//Por ello, debemos programar la simulación para que pare cuando deseemos
	Simulator::Stop(tstop + Seconds(1));

	//Lanzamos la simulación
	Simulator::Run ();

	Simulator::Destroy ();

	//Cogemos los datos y los añadimos a las estructuras
	resultados->tiempo_parada = obs.GetIntervaloParada().GetSeconds();
	

	if(medirEnl==1){
		resultados->bytes_enlace[0] = double(enl1.DevuelveBytes());
		resultados->bytes_enlace[1] = double(enl2.DevuelveBytes());

		if(version == 1)
		{
			resultados->bytes_enlace[3] = double(enl4.DevuelveBytes());		
			resultados->bytes_enlace[4] = double(enl5.DevuelveBytes());
			resultados->bytes_enlace[5] = double(enl6.DevuelveBytes());
		}
	}
}
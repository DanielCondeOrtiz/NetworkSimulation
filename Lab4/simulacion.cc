/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Practica 04 - Simulación de Redes
Fecha: 19/11/17
Alumnos: Daniel Conde Ortiz
		 Miguel Ruiz Ramos

Fichero: simulacion.cc
Descripcion: Este fichero contiene la funcion main y la funcion simulacion, encargadas de configurar el sistema, generar las graficas y ejecutar la simulacion
*/

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "Observador.h"
#include "ns3/point-to-point-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include <ns3/gnuplot.h>
#include <sstream>

using namespace ns3;
#define NCSMA     20
#define RETARDOB   "6560ns"
#define RETARDOE   "2ms"
#define TASAB      "100Mb/s"
#define TASAE      "2Mb/s"
#define TASAFUENTE "64Kb/s"
#define TAMANO    780
#define TON			"150ms"
#define TOFF		"650ms"
#define TSALTO		"50ms"
#define STOP      "50s"
#define RUN       "1s"
#define TSTUDENT  2.145
#define MAXTIEMPO 5
#define MUESTRAS 15
#define MAXCOLA 5 //En paquetes

NS_LOG_COMPONENT_DEFINE ("practica04");

//Estructura para acumular los valores de la simulacion
struct acumuladores {
	Average<double> acumCorrectos;
	Average<double> acumRetardos;
};

void simulacion (uint32_t nCsma, Time delayb, DataRate rateb, Time delaye, DataRate ratee, Time ton, Time toff, uint32_t packetSize, DataRate ratef, Time tstop, Time tstart, uint32_t tamanocola, acumuladores * datos);


int main (int argc, char *argv[])
{
	//Se establece la resolucion en nano segundos
	Time::SetResolution (Time::NS);

	GlobalValue::Bind ("ChecksumEnabled", BooleanValue(true));
	
	//Se establecen los valores por defecto para las variables
	uint32_t    nCsma = NCSMA;	 
	Time        delayb (RETARDOB);
	Time        delaye (RETARDOE);
	DataRate    rateb (TASAB);
	DataRate    ratee (TASAE);
	DataRate    ratef (TASAFUENTE);
	uint32_t    packetSize = TAMANO;
	Time        tstop (STOP);
	Time        tstart (RUN);
	Time		ton (TON);
	Time 		toff (TOFF);
	Time        pasoTiempo(TSALTO);

	//Para configurar los valores de las variables mediante linea de comandos
	CommandLine cmd;

	cmd.AddValue ("ton", "tiempo del periodo de la fuente en on", ton);
	cmd.AddValue ("toff", "tiempo del periodo de la fuente en off", toff);
	cmd.AddValue ("tamPqt", "tamano de las SDU de aplicacion de la fuente", packetSize);
	cmd.AddValue ("tasaEnvio", "tasa de bits durante el periodo en on", ratef);
	//cmd.AddValue ("tcola", "tamano de la cola de la puerta de enlace", tamanocola);

	cmd.AddValue ("nCsma", "número de nodos en el bus", nCsma);
	cmd.AddValue ("retardoBus", "retardo de propagación en el bus", delayb);
	cmd.AddValue ("capacidadBus", "capacidad del bus", rateb);
	cmd.AddValue ("retardoEn", "retardo de propagación en el enlace P2P", delaye);
	cmd.AddValue ("capacidadEn", "capacidad del enlace P2P", ratee);

	cmd.AddValue ("tstart", "instante de simulacion de inicio de las aplicaciones cliente", tstart);
	cmd.AddValue ("tstop", "intante de simulacion de fin de aplicaciones cliente y del servidor", tstop);

	cmd.Parse (argc, argv);

	// Debemos garantizar que hay al menos un cliente y el servidor.
	if (nCsma <= 1)
		nCsma = 2;

	//Para añadir el tamaño de la cola al nombre de la linea
	std::ostringstream strs1;
	std::string nodos;
	std::string tasafuentes;
	std::string tasabus;
	std::string tasap2p;
	std::string retardobus;
	std::string retardop2p;
	std::string tamanopaq;

	strs1 << nCsma;
	nodos = strs1.str();
	strs1.str("");

	strs1 << ratef.GetBitRate();
	tasafuentes = strs1.str();
	strs1.str("");
	
	strs1 << rateb.GetBitRate();
	tasabus = strs1.str();
	strs1.str("");

	strs1 << ratee.GetBitRate();
	tasap2p = strs1.str();
	strs1.str("");

	strs1 << delayb;
	retardobus = strs1.str();
	strs1.str("");

	strs1 << delaye;
	retardop2p = strs1.str();
	strs1.str("");

	strs1 << packetSize;
	tamanopaq = strs1.str();
	strs1.str("");


	//Grafica de paquetes correctos
	Gnuplot Graf_Correctos;
	Graf_Correctos.SetLegend ("Tiempo medio de permanencia de las fuentes en el estado On (ms)", "Porcentaje de Paq. Correctos(%)");
	Graf_Correctos.SetTitle ("Porcentaje de paquetes correctamente transmitidos segun la cola critica y periodo de on con: \\n"
		+ nodos + " nodos en el bus nCsma ejecutando una aplicacion on/off" +
		"\\nTasa de bits en el bus: "+ tasabus +"bps; Retardo en el bus: " + retardobus +
		"\\nTasa de bits en el enlace punto a punto: "+ tasap2p + "bps; Retardo en el enlace: "+ retardop2p +
		"\\nTasa de envio de la fuente: " + tasafuentes +"bps; Tamano de los paquetes: "+ tamanopaq+"bytes");

	//Grafica de retardo
	Gnuplot Graf_Retardo;
	Graf_Retardo.SetLegend ("Tiempo medio de permanencia de las fuentes en el estado On (ms)", "Retardo medio(ms)");
	Graf_Retardo.SetTitle ("Tiempo medio hasta recepcion del paquete segun la cola critica y periodo de on con \\n"
		+ nodos + " nodos en el bus nCsma ejecutando una aplicacion on/off" +
		"\\nTasa de bits en el bus: "+ tasabus +"bps; Retardo en el bus: " + retardobus +
		"\\nTasa de bits en el enlace punto a punto: "+ tasap2p + "bps; Retardo en el enlace: "+ retardop2p +
		"\\nTasa de envio de la fuente: " + tasafuentes +"bps; Tamano de los paquetes: "+ tamanopaq+"bytes");

	struct acumuladores acum;
	Time t_on_inicial = ton;

	//Bucle para cada linea, cambiando el tamaño de la cola
	for(uint32_t tamcola = 1; tamcola <= MAXCOLA; tamcola++)
	{
		//Para añadir el tamaño de la cola al nombre de la linea
		std::ostringstream strs2;
		std::string str2;

		strs2 << tamcola;
		str2 = strs2.str();

		//Creamos las dos lineas y seleccionamos su estilo
		Gnuplot2dDataset datos_medias2("Tamaño de cola " + str2);
		Gnuplot2dDataset datos_medias1("Tamaño de cola " + str2);

		datos_medias1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datos_medias1.SetErrorBars (Gnuplot2dDataset::Y);

		datos_medias2.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datos_medias2.SetErrorBars (Gnuplot2dDataset::Y);

		NS_LOG_INFO("Tamaño cola: " << tamcola << " paquete(s)");	

		//Bucle para cada punto en el eje de abscisas, cambiando el tiempo en on de las fuentes
		for (uint32_t tiempoon = 0; tiempoon < MAXTIEMPO; tiempoon++)
		{
			ton = t_on_inicial + pasoTiempo * tiempoon;
			NS_LOG_INFO("Ton: " << ton.GetMilliSeconds() << "ms");	
			//Bucle para simular MUESTRAS veces para el mismo valor de tiempo on
			for (uint32_t sim = 0; sim < MUESTRAS; sim++)
			{
				NS_LOG_DEBUG("\nSimulacion: " << sim +1);
				simulacion(nCsma, delayb, rateb, delaye, ratee, ton, toff, packetSize, ratef, tstop, tstart, tamcola, &acum);						

			}

			//Variables de error para los diferentes datos recolectados de las simulaciones
			double error1 = TSTUDENT * sqrt (acum.acumCorrectos.Var () / acum.acumCorrectos.Count ());
			datos_medias1.Add (ton.GetMilliSeconds(), acum.acumCorrectos.Mean(), error1);


			NS_LOG_INFO("Paquetes correctos: " <<  acum.acumCorrectos.Mean() << "%");	

			double error2 = TSTUDENT * sqrt (acum.acumRetardos.Var () / acum.acumRetardos.Count ());
			datos_medias2.Add (ton.GetMilliSeconds(), acum.acumRetardos.Mean(), error2);

			NS_LOG_INFO("Retardo medio: " << acum.acumRetardos.Mean() << "ms");

			acum.acumCorrectos.Reset();
			acum.acumRetardos.Reset();
			
		}

		// Añadimos las curvas a las gráficas
		Graf_Correctos.AddDataset (datos_medias1);
		Graf_Retardo.AddDataset (datos_medias2);
	}

	NS_LOG_INFO("La simulacion ha acabado, generando los archivos con las graficas");

	// Para terminar generamos los ficheros
	std::ofstream fichero_01 ("practica04-01.plt");
	Graf_Correctos.GenerateOutput (fichero_01);
	fichero_01 << "pause -1" << std::endl;
	fichero_01.close ();


	std::ofstream fichero_02 ("practica04-02.plt");
	Graf_Retardo.GenerateOutput (fichero_02);
	fichero_02 << "pause -1" << std::endl;
	fichero_02.close ();

	return 0;
}

//Funcion encargada de configurar el escenario y ejecutar la simulación.
void simulacion (uint32_t nCsma, Time delayb, DataRate rateb, Time delaye, DataRate ratee, Time ton, Time toff, uint32_t packetSize, DataRate ratef, Time tstop, Time tstart, uint32_t tamanocola, acumuladores * datos)
{
	// Aquí empieza el escenario
	

	// Nodos que pertenecen al enlace punto a punto
	NodeContainer nodosP2P;
	nodosP2P.Create (2);

	// Nodos que pertenecen a la red de área local
	// Como primer nodo añadimos el encaminador que proporciona acceso
	// al enlace punto a punto.
	NodeContainer nodosCsma;
	nodosCsma.Add (nodosP2P.Get (1));
	nodosCsma.Create (nCsma-1);

	//Asignamos valores de atributos a nodos y canales
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", DataRateValue(rateb));
	csma.SetChannelAttribute ("Delay", TimeValue (delayb));

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install (nodosCsma);
	
	PointToPointHelper p2p;
	p2p.SetChannelAttribute("Delay",TimeValue (delaye));
	p2p.SetDeviceAttribute("DataRate", DataRateValue(ratee));
	
	NetDeviceContainer p2pDevices;
	p2pDevices = p2p.Install(nodosP2P); 

	// Instalamos la pila TCP/IP en todos los nodos
	InternetStackHelper stack;
	stack.Install (nodosCsma);
	stack.Install (nodosP2P.Get(0));
	
	// Y les asignamos direcciones
	Ipv4AddressHelper busAddress;
	busAddress.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4AddressHelper p2pAddress;
	p2pAddress.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaInterfaces = busAddress.Assign (csmaDevices);
	Ipv4InterfaceContainer p2pInterfaces = p2pAddress.Assign (p2pDevices);

    // Cálculo de rutas
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	NS_LOG_DEBUG("Escenario configurado");
	// Configuramos el tráfico	
	// Instalación de las aplicaciones
	uint16_t sinkPort = 10000;

	//Helper para el sumidero
	PacketSinkHelper sumidero ("ns3::UdpSocketFactory",InetSocketAddress(p2pInterfaces.GetAddress(0),sinkPort));

	//Helper para las fuentes
	OnOffHelper fuente("ns3::UdpSocketFactory",InetSocketAddress(p2pInterfaces.GetAddress(0),sinkPort));

	//Obtenemos los valores de ton y toff que van a tener las fuentes
	std::ostringstream strs1;
	std::ostringstream strs2;
	std::string str;

	strs1 << ton.GetSeconds();
	str = strs1.str();

	std::string ontime = "ns3::ExponentialRandomVariable[Mean="+ str +"]";
	
	strs2 << toff.GetSeconds();
	str = strs2.str();

	std::string offtime = "ns3::ExponentialRandomVariable[Mean="+ str +"]";

	//Se los asignamos, junto a otros atributos
	fuente.SetAttribute("OnTime", StringValue(ontime));
	fuente.SetAttribute("OffTime", StringValue(offtime));
	fuente.SetAttribute("DataRate", DataRateValue(ratef));
	fuente.SetAttribute("PacketSize", UintegerValue(packetSize));


	//Asignamos los tiempos de start y stop
	NodeContainer fuentes;

	for (uint32_t i = 1; i < nCsma; i++)
		fuentes.Add (nodosCsma.Get (i));

	ApplicationContainer fuentesApp = fuente.Install(fuentes);

	fuentesApp.Start(tstart);
	fuentesApp.Stop(tstop);


	ApplicationContainer sumideroApp = sumidero.Install(nodosP2P.Get(0));

	sumideroApp.Start(tstart);
	sumideroApp.Stop(tstop);

	//Asignamos el tamaño de la cola
	p2pDevices.Get(1)->GetObject<PointToPointNetDevice> ()->GetQueue()->GetObject<DropTailQueue>()->SetAttribute("MaxPackets", UintegerValue(tamanocola));

	NS_LOG_DEBUG("Aplicaciones configuradas");

	//////////Aquí empieza la parte de análisis
	Observador observador1 (&csmaDevices, p2pDevices.Get (0)->GetObject<PointToPointNetDevice> (), &sumideroApp);

	NS_LOG_DEBUG("Observador creado");

    // Lanzamos la simulacion
	Simulator::Run ();

    // Finalizamos el proceso de simulacion
	Simulator::Destroy ();

	//Obtenemos los valores de la simulacion y los guardamos en los acumuladores.
	double retardo = (observador1.GetRetardoMedio());		
	datos->acumRetardos.Update(retardo);

	double correctos = observador1.GetPaqCorrectos();
	datos->acumCorrectos.Update(correctos);

	NS_LOG_DEBUG("Datos introducidos en la estructura  Struct acumuladores datos");
}
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Practica 05 - Simulación de Redes
Fecha: 02/12/17
Alumnos: Daniel Conde Ortiz
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
#include "Observador.h"
#include "ns3/point-to-point-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include <ns3/gnuplot.h>
#include <sstream>

using namespace ns3;
#define NCSMA     	5
#define RETARDOB   "6560ns"
#define RETARDOE   "2ms"
#define TASAB      "10Mb/s"
#define TASAE      "2Mb/s"
#define TASAFUENTE "1Mb/s"
#define TAMANO    1410
#define TON			"1s"
#define TOFF		"0s"
#define STOP      "20s"
#define RUN       "1s"
#define TSTUDENT  1.7823
#define MUESTRAS 13
#define ERROR 0.00001
#define NUM_SALTOS 5
#define SALTO 0.005

NS_LOG_COMPONENT_DEFINE ("practica05");

double escenario (uint32_t nCsma, Time delayb, DataRate rateb, Time delaye, DataRate ratee, Time ton, Time toff, uint32_t packetSize, DataRate ratef, Time tstop,
	Time tstart, double perror, uint32_t metodo, Gnuplot2dDataset ** datos_medias1, uint32_t flag_grafica1);


int main (int argc, char *argv[])
{
	//Se establece la resolucion en nanosegundos
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
	double		perror_inicial = ERROR;
	double		salto = SALTO;
	uint32_t	flag_grafica1=0;
	
	//Para configurar los valores de las variables mediante linea de comandos
	CommandLine cmd;

	cmd.AddValue ("ton", "tiempo del periodo de la fuente en on", ton);
	cmd.AddValue ("toff", "tiempo del periodo de la fuente en off", toff);
	cmd.AddValue ("tamPqt", "tamano de las SDU de aplicacion de la fuente", packetSize);
	cmd.AddValue ("tasaEnvio", "tasa de bits durante el periodo en on", ratef);

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
	std::string tasafuentes;
	std::string tamanopaq;


	strs1 << ratef.GetBitRate();
	tasafuentes = strs1.str();
	strs1.str("");

	strs1 << packetSize;
	tamanopaq = strs1.str();
	strs1.str("");

	
	//Grafica de rendimiento de protocolo TCP
	Gnuplot Graf_Rendimiento;
	Graf_Rendimiento.SetLegend ("Probabilidad de Error", "Octetos recibidos/tiempo consumido (byte/s)");
	Graf_Rendimiento.SetTitle ("Número de octetos recibidos por unidad de tiempo, segun probabilidad de error y metodo tcp con:");
		// + "\\nTasa de envio de la fuente: " + tasafuentes + "bps; Tamano de los paquetes: "+ tamanopaq +"bytes");
	
	//Grafica de ventana de congestión
	Gnuplot Graf_Ventana;
	Graf_Ventana.SetLegend ("Tiempo (ms)", "Ventana de Congestion");
	Graf_Ventana.SetTitle ("Ventana de congestión del socket segun metodo TCP con" );
		//+ "\\nTasa de envio de la fuente: " + tasafuentes + "bps; Tamano de los paquetes: "+ tamanopaq +"bytes");

	//Elegimos una ventana entre los 5 y 10 segundos de la simulación para representar cómo funcionaría en general
	//cada protocolo TCP respecto al tamaño de la ventana de congestión
	Graf_Ventana.AppendExtra("set xrange [7000:12000]");
	

	//Bucle para cada linea, cambiando el método tcp
	for(uint32_t metodo = 1; metodo <= 3; metodo++)
	{
		//Elegimos el metodo TCP de la simulación mediante un switch y su nombre en string
		std::string str2;
		switch(metodo)
		{
			case 1:
			str2 = "NewReno";
			break;

			case 2:
			str2 = "Illinois";
			break;

			case 3:
			str2 = "Veno";
			break;
		}

		NS_LOG_INFO("\nMetodo TCP: " + str2);
		
		//Creamos las dos lineas y seleccionamos su estilo
		//Datos medias1 es un puntero porque se genera en el observador durante la simulacion
		Gnuplot2dDataset * datos_medias1;

		Gnuplot2dDataset datos_medias2("Metodo TCP: " + str2);
		
		datos_medias2.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datos_medias2.SetErrorBars (Gnuplot2dDataset::Y);
		

		//Bucle para cada punto en el eje de abscisas de la segunda grafica, cambiando probabilidad de error
		for (double prob_error = perror_inicial; prob_error <= perror_inicial + NUM_SALTOS * salto; prob_error += salto)
		{
			
			Average<double> acumRetardos;
			NS_LOG_INFO("\nProbabilidad de error: " << prob_error);

			//Bucle para simular MUESTRAS veces para la misma probabilidad de error
			for (uint32_t sim = 0; sim < MUESTRAS; sim++)
			{
				NS_LOG_INFO("\nSimulacion: " << sim +1);

				//La primera simulacion de la ultima probabilidad de error, obtenemos la gráfica 1
				if(sim == 0 && prob_error == (perror_inicial + (NUM_SALTOS-1) * salto))
					flag_grafica1 = 1;
				else
					flag_grafica1 = 0;


				double tmp = escenario(nCsma, delayb, rateb, delaye, ratee, ton, toff, packetSize, ratef, tstop, tstart, prob_error, metodo, &datos_medias1, flag_grafica1);						
				
				acumRetardos.Update(tmp);

				//Cuando hemos obtenidos los datos de la ventana de congestion, le ponemos el nombre a la linea y la añadimos a su grafica
				if(flag_grafica1 == 1)
				{
					datos_medias1->SetTitle("Metodo TCP: " + str2);
					datos_medias1->SetStyle (Gnuplot2dDataset::LINES_POINTS);
					Graf_Ventana.AddDataset (*datos_medias1);
				}
			}

			//Variable de error para los diferentes datos recolectados de las simulaciones
			
			double error2 = TSTUDENT * sqrt (acumRetardos.Var () / acumRetardos.Count ());
			datos_medias2.Add (prob_error, acumRetardos.Mean(), error2);
			
		}

		// Añadimos la curva de rendimiento a las gráfica 2
		Graf_Rendimiento.AddDataset (datos_medias2);
	}

	NS_LOG_INFO("La simulacion ha acabado, generando los archivos con las graficas");

	// Para terminar generamos los ficheros
	std::ofstream fichero_01 ("practica05-1.plt");
	Graf_Ventana.GenerateOutput (fichero_01);
	fichero_01 << "pause -1" << std::endl;
	fichero_01.close ();
	
	std::ofstream fichero_02 ("practica05-2.plt");
	Graf_Rendimiento.GenerateOutput (fichero_02);
	fichero_02 << "pause -1" << std::endl;
	fichero_02.close ();
	
	return 0;
}

//Funcion encargada de configurar el escenario y ejecutar la simulación.
double escenario (uint32_t nCsma, Time delayb, DataRate rateb, Time delaye, DataRate ratee, Time ton, Time toff, uint32_t packetSize, DataRate ratef, Time tstop,
	Time tstart, double perror, uint32_t metodo, Gnuplot2dDataset ** datos_medias1, uint32_t flag_grafica1)
{
	// Aquí empieza el escenario
	//Configuramos el método TCP que queremos usar durante la simulacion
	switch(metodo){
		case 1:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
		break;
		
		case 2:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpIllinois"));
		break;
		
		case 3:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpVeno"));
		break;
	}
	
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
	Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
	errores->SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
	errores->SetRate (perror);
	p2p.SetDeviceAttribute("ReceiveErrorModel", PointerValue(errores));
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
	PacketSinkHelper sumidero ("ns3::TcpSocketFactory",InetSocketAddress(p2pInterfaces.GetAddress(0),sinkPort));

	//Helper para las fuentes
	OnOffHelper fuente("ns3::TcpSocketFactory",InetSocketAddress(p2pInterfaces.GetAddress(0),sinkPort));

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


	//Asignamos los tiempos de start y stop
	NodeContainer fuentes;

	fuentes.Add (nodosCsma.Get(nCsma-1));
	ApplicationContainer fuentesApp = fuente.Install(fuentes);

	fuentesApp.Start(tstart);
	fuentesApp.Stop(tstop);


	ApplicationContainer sumideroApp = sumidero.Install(nodosP2P.Get(0));

	sumideroApp.Start(tstart);

	NS_LOG_DEBUG("Aplicaciones configuradas");


	//////////Aquí empieza la parte de análisis
	Observador observador1 (csmaDevices.Get(nCsma-1)->GetObject<CsmaNetDevice>(), p2pDevices.Get (0)->GetObject<PointToPointNetDevice> (), &sumideroApp, &fuentesApp, flag_grafica1, tstop);

	NS_LOG_DEBUG("Observador creado");

    // Lanzamos la simulacion
	Simulator::Run ();

    // Finalizamos el proceso de simulacion
	Simulator::Destroy ();

	NS_LOG_DEBUG("Datos introducidos en el acumulador");

	//Obtenemos la grafica si esta es la simulacion que hemos decidido que vamos a representar
	if (flag_grafica1 == 1)
	{
		*datos_medias1 = observador1.GetDatos();
	}

	//Obtenemos los valores de la simulacion y los guardamos en el acumulador.		
	double octetosRx = observador1.GetOctetosRecibidos()/tstop.GetSeconds();
	NS_LOG_INFO("Numero de octetos Recibidos: " << octetosRx);
	
	return octetosRx;
	
}